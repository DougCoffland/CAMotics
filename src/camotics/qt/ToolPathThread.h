/******************************************************************************\

    CAMotics is an Open-Source CAM software.
    Copyright (C) 2011-2015 Joseph Coffland <joseph@cauldrondevelopment.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

#ifndef CAMOTICS_TOOL_PATH_THREAD_H
#define CAMOTICS_TOOL_PATH_THREAD_H

#include "CutSimThread.h"

#include <camotics/cutsim/ToolPath.h>
#include <camotics/sim/ToolTable.h>

#include <string>
#include <vector>


namespace CAMotics {
  class ToolTable;
  class ToolPath;
  class Project;

  class ToolPathThread : public CutSimThread {
    ToolTable tools;
    std::vector<std::string> files;
    cb::SmartPointer<ToolPath> path;

    public:
    ToolPathThread(cb::Application &app, int event, QWidget *parent,
                   const cb::SmartPointer<CutSim> &cutSim,
                   const cb::SmartPointer<Project> &project);

    const cb::SmartPointer<ToolPath> &getPath() const {return path;}

    // From cb::Thread
    void run();
  };
}

#endif // CAMOTICS_TOOL_PATH_THREAD_H

