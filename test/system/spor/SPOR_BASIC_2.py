#!/usr/bin/env python3

import sys
import os
from itertools import product

import TEST
import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_IBOFOS

current_test = 0
volId = 1

############################################################################
## Test Description
##  write pattern to the volume, simulate SPOR, verify the pattern
##  and write and verify another pattern to see pos works properly
############################################################################
def test(offset, size):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    TEST_LIB.create_new_pattern(volId)
    TEST_FIO.write(volId, offset, size, TEST_LIB.get_latest_pattern(volId))

    TEST_SETUP_IBOFOS.trigger_spor()
    TEST_SETUP_IBOFOS.dirty_bringup()

    TEST_SETUP_IBOFOS.create_subsystem(volId)
    TEST_SETUP_IBOFOS.mount_volume(volId)

    TEST_FIO.verify(volId, offset, size, TEST_LIB.get_latest_pattern(volId))

    TEST_LIB.create_new_pattern(volId)
    TEST_FIO.write(volId, offset, size, TEST_LIB.get_latest_pattern(volId))

    TEST_FIO.verify(volId, offset, size, TEST_LIB.get_latest_pattern(volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

def execute():
    offsets = [0, 4096]
    sizes = ['128k', '256k']

    if TEST.quick_mode == False:
        for (_offset, _size) in product(offsets, sizes):
            test(offset=_offset, size=_size)
    else:
        test(offset=offsets[-1], size=sizes[-1])

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_IBOFOS.clean_bringup()

    TEST_SETUP_IBOFOS.create_subsystem(volId)
    TEST_SETUP_IBOFOS.create_volume(volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)