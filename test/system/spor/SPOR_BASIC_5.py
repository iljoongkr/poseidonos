#!/usr/bin/env python3

import sys
import os
import re
import subprocess

from threading import Thread

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_IBOFOS

volId = 1
current_test = 0

############################################################################
## Test Description
##  Multi threads simultaneously write patterns to the volume,
##  simulate SPOR,
##  and verify all patterns to see pos works properly
############################################################################
def test(size):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    write_size = TEST_LIB.parse_size(size)
    max_num_thread = int(TEST_LIB.get_num_thread())
    thread_list = []

    for idx in range(max_num_thread):
        TEST_LIB.create_new_pattern(volId)
        th = Thread(target=TEST_FIO.write, args=(volId, write_size * idx, size, TEST_LIB.get_latest_pattern(volId)))
        thread_list.append(th)
        th.start()

    for th in thread_list:
        th.join()

    TEST_SETUP_IBOFOS.trigger_spor()
    TEST_SETUP_IBOFOS.dirty_bringup()

    TEST_SETUP_IBOFOS.create_subsystem(volId)
    TEST_SETUP_IBOFOS.mount_volume(volId)

    for idx in range(max_num_thread):
        TEST_FIO.verify(volId, write_size * idx, size, TEST_LIB.get_pattern(volId, idx))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

def execute():
    sizes = ['1M']
    for _size in sizes:
        test(size=_size)

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_IBOFOS.clean_bringup()

    TEST_SETUP_IBOFOS.create_subsystem(volId)
    TEST_SETUP_IBOFOS.create_volume(volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)
