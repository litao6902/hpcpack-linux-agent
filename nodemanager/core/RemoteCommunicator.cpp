#include <sstream>
#include <stdlib.h>

#include "RemoteCommunicator.h"
#include "../utils/String.h"
#include "../utils/System.h"
#include "../arguments/StartJobAndTaskArgs.h"
#include "../common/ErrorCodes.h"

using namespace web::http;
using namespace web;
using namespace hpc::utils;
using namespace hpc::arguments;
using namespace hpc::core;
using namespace hpc::common;

RemoteCommunicator::RemoteCommunicator(const std::string& networkName, IRemoteExecutor& exec) :
    listeningUri(this->GetListeningUri(networkName)), isListening(false), executor(exec), listener(listeningUri)
{
    this->listener.support(
        methods::POST,
        [this](auto request) { this->HandlePost(request); });

    this->processors["startjobandtask"] = [this] (const auto& j, const auto& c) { return this->StartJobAndTask(j, c); };
    this->processors["starttask"] = [this] (const auto& j, const auto& c) { return this->StartTask(j, c); };
    this->processors["endjob"] = [this] (const auto& j, const auto& c) { return this->EndJob(j, c); };
    this->processors["endtask"] = [this] (const auto& j, const auto& c) { return this->EndTask(j, c); };
    this->processors["ping"] = [this] (const auto& j, const auto& c) { return this->Ping(j, c); };
    this->processors["metric"] = [this] (const auto& j, const auto& c) { return this->Metric(j, c); };
}

RemoteCommunicator::~RemoteCommunicator()
{
    this->Close();
}

void RemoteCommunicator::Open()
{
    if (!this->isListening)
    {
        this->listener.open().then([this](auto t)
        {
            this->isListening = !IsError(t);
            Logger::Info(
                "Opening at {0}, result {1}",
                this->listener.uri().to_string(),
                this->isListening ? "opened." : "failed.");

            if (!this->isListening)
            {
                exit((int)ErrorCodes::FailedToOpenPort);
            }
        });
    }
}

void RemoteCommunicator::Close()
{
    if (this->isListening)
    {
        try
        {
            this->listener.close().wait();
            this->isListening = false;
        }
        catch (const std::exception& ex)
        {
            Logger::Error("Exception happened while close the listener {0}, {1}", this->listener.uri().to_string().c_str(), ex.what());
        }
    }
}

void RemoteCommunicator::HandlePost(http_request request)
{
    auto uri = request.relative_uri().to_string();
    Logger::Info("Request: Uri {0}", uri);

    std::vector<std::string> tokens = String::Split(uri, '/');

    if (tokens.size() < 4)
    {
        Logger::Warn("Not supported uri {0}", uri);
        return;
    }

    // skip the first '/'.
    int p = 1;
    auto apiSpace = tokens[p++];
    auto nodeName = tokens[p++];
    auto methodName = tokens[p++];

    Logger::Debug("Request: Uri {0}, Node {1}, Method {2}", uri, nodeName, methodName);

    if (apiSpace != ApiSpace)
    {
        Logger::Error("Not allowed ApiSpace {0}", apiSpace);
        request.reply(status_codes::NotFound, U("Not found"))
            .then([this](auto t) { this->IsError(t); });
        return;
    }

    std::string callbackUri;
    auto callbackHeader = request.headers().find(CallbackUriKey);
    if (callbackHeader != request.headers().end())
    {
        callbackUri = callbackHeader->second;
        Logger::Debug("CallbackUri found {0}", callbackUri.c_str());
    }

    auto processor = this->processors.find(methodName);

    if (processor != this->processors.end())
    {
        request.extract_json().then([processor, callback = std::move(callbackUri)](pplx::task<json::value> t)
        {
            auto j = t.get();
            Logger::Debug("Json: {0}", j.serialize());

            return processor->second(j, callback);
        })
        .then([request, this](pplx::task<json::value> t)
        {
            std::string errorMessage;
            if (!this->IsError(t, errorMessage))
            {
                auto jsonBody = t.get();
                request.reply(status_codes::OK, jsonBody).then([this](auto t) { this->IsError(t); });
            }
            else
            {
                request.reply(status_codes::InternalError, errorMessage).then([this](auto t) { this->IsError(t); });
            }
        });
    }
    else
    {
        Logger::Warn("Unable to find the method {0}", methodName.c_str());
        request.reply(status_codes::NotFound, "").then([this](auto t) { this->IsError(t); });
    }
}

json::value RemoteCommunicator::StartJobAndTask(const json::value& val, const std::string& callbackUri)
{
    auto args = StartJobAndTaskArgs::FromJson(val);
    return this->executor.StartJobAndTask(std::move(args), callbackUri);
}

json::value RemoteCommunicator::StartTask(const json::value& val, const std::string& callbackUri)
{
    auto args = StartTaskArgs::FromJson(val);
    return this->executor.StartTask(std::move(args), callbackUri);
}

json::value RemoteCommunicator::EndJob(const json::value& val, const std::string& callbackUri)
{
    auto args = EndJobArgs::FromJson(val);
    return this->executor.EndJob(std::move(args));
}

json::value RemoteCommunicator::EndTask(const json::value& val, const std::string& callbackUri)
{
    auto args = EndTaskArgs::FromJson(val);
    return this->executor.EndTask(std::move(args), callbackUri);
}

json::value RemoteCommunicator::Ping(const json::value& val, const std::string& callbackUri)
{
    return this->executor.Ping(callbackUri);
}

json::value RemoteCommunicator::Metric(const json::value& val, const std::string& callbackUri)
{
    return this->executor.Metric(callbackUri);
}

const std::string RemoteCommunicator::ApiSpace = "api";
const std::string RemoteCommunicator::CallbackUriKey = "CallbackURI";

std::string RemoteCommunicator::GetListeningUri(const std::string& networkName)
{
    return String::Join("", "http://", System::GetIpAddress(IpAddressVersion::V4, networkName), ":50000");
}

