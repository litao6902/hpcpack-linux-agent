#!/bin/bash

. common.sh

[ -z "$1" ] && echo "task id not specified" && exit 2
[ -z "$2" ] && echo "affinity not specified" && exit 2

taskId=$1

if $CGInstalled; then
	groupName=$(GetCGroupName $taskId)
	group=$CGroupSubSys:$groupName

	maxLoop=3
	while [ $maxLoop -gt 0 ]
	do
		cgcreate -g $group
		ec=$?
		if [ $ec -eq 0 ]
		then
			break
		fi

		echo "Failed to create cgroup $group, error code $ec, retry after .5 seconds"
		((maxLoop--))
		sleep .5
	done

	if [ $ec -ne 0 ]
	then
		exit $ec
	fi

	maxLoop=3
	while [ $maxLoop -gt 0 ]
	do
		echo "$2" > $CGroupRoot/cpuset/$groupName/cpuset.cpus
		ec=$?
		if [ $ec -eq 0 ]
		then
			break
		fi

		echo "Failed to set cpus for $group, error code $ec, retry after .5 seconds"
		((maxLoop--))
		sleep .5
	done

	if [ $ec -ne 0 ]
	then
		exit $ec
	fi

	maxLoop=3
	while [ $maxLoop -gt 0 ]
	do
		echo 0 > $CGroupRoot/cpuset/$groupName/cpuset.mems
		ec=$?
		if [ $ec -eq 0 ]
		then
			break
		fi

		echo "Failed to set mems for $group, error code $ec, retry after .5 seconds"
		((maxLoop--))
		sleep .5
	done	

	if [ $ec -ne 0 ]
	then
		exit $ec
	fi
fi

