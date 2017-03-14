#!/usr/bin/python
"""Example usage of Python script with PBala"""

# Job parallelizer in PVM for SPMD executions in computing cluster
# URL: https://github.com/oscarsaleta/PVMantz
#
# Copyright (C) 2016  Oscar Saleta Reig
#
# PBala is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# PBala is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with PBala.  If not, see <http://www.gnu.org/licenses/>.

import sys

# We define a main function that receives an integer taskId
# and a vector taskArgs. Within this function, we can perform
# any operation on these variables. We will just print them
def main(task_id, task_args):
    """Example main function"""
    print task_id
    print task_args

# Now we tell Python that when this program is executed, we want
# to execute the main() function and pass it sys.argv[1] (the first
# number of the datafile) as taskId, and sys.argv[2:] (every other
# number of the datafile) as taskArgs.
if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
