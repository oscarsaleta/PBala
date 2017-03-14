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

# Using Maple, we already have the arguments defined!
# taskId is defined as the first number of the row
# taskArgs is a vector that contains all the other arguments
# In order to use them, we just need to use these variables

print("Arguments passed are:",taskId,taskArgs);