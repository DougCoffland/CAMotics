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

#include "ToolTable.h"

#include <camotics/gcode/Parser.h>
#include <camotics/gcode/ast/Block.h>
#include <camotics/gcode/ast/Word.h>

#include <cbang/Exception.h>
#include <cbang/String.h>

#include <cbang/log/Logger.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/json/Sink.h>

#include <cbang/xml/XMLProcessor.h>
#include <cbang/xml/XMLWriter.h>

#include <cctype>
#include <vector>

using namespace std;
using namespace cb;
using namespace CAMotics;


ToolTable::ToolTable() : current(-1) {
  clear();
}


bool ToolTable::has(unsigned tool) const {
  return find(tool) != end();
}


const Tool &ToolTable::get(unsigned tool) const {
  const_iterator it = find(tool);
  if (it != end()) return it->second;

  THROWS("Non-existant tool " << tool);
}


Tool &ToolTable::get(unsigned tool) {
  iterator it = find(tool);
  if (it != end()) return it->second;

  LOG_WARNING("Auto-creating non-existant tool " << tool);
  return (*this)[tool] = Tool(tool);
}


void ToolTable::add(const Tool &tool) {
  if (!insert(value_type(tool.getNumber(), tool)).second)
    THROWS("Tool with number " << tool.getNumber()
           << " already in tool table");

  LOG_INFO(3, "Added tool " << tool.getNumber() << " with radius "
           << tool.getRadius());
}


void ToolTable::clear() {
  map<unsigned, Tool>::clear();
  add(Tool::null); // Tool zero
}


void ToolTable::operator()(const SmartPointer<Block> &block) {
  Evaluator eval;

  // Evaluate expressions
  for (Block::iterator it = block->begin(); it != block->end(); it++)
    (*it)->eval(eval);

  Word *T = block->findWord('T');
  Word *P = block->findWord('P');
  Word *D = block->findWord('D');

  if (T) {
    unsigned number = T->getValue();
    unsigned pocket = P ? P->getValue() : number;

    Tool tool(number, pocket);

    if (D) tool.setRadius(D->getValue() / 2);

    for (const char *c = "XYZABCUVWIJQ"; *c; c++) {
      Word *word = block->findWord(*c);
      if (word) tool.set(*c, word->getValue());
    }

    // Add tool
    add(tool);
  }
}


XMLHandler *ToolTable::getHandler(XMLProcessor &processor,
                                  const XMLAttributes &attrs) {
  processor.pushContext();
  clear(); // New tool table
  return this;
}


void ToolTable::freeHandler(XMLProcessor &processor, XMLHandler *handler) {
  processor.popContext();
}


void ToolTable::startElement(const string &name, const XMLAttributes &attrs) {
  if (name == "tool") {
    unsigned number;
    if (attrs.has("number")) number = String::parseU32(attrs["number"]);
    else number = size();

    Tool tool(number);
    current = number;
    tool.read(attrs);

    add(tool);
  }
}


void ToolTable::endElement(const std::string &name) {
  current = -1;
}


void ToolTable::text(const std::string &text) {
  if (0 <= current) {
    string desc = String::trim(text);
    if (!desc.empty()) {
      Tool &tool = get(current);

      if (!tool.getDescription().empty())
        desc = tool.getDescription() + " " + desc;

      tool.setDescription(desc);
    }
  }
}


void ToolTable::write(XMLWriter &writer) const {
  writer.startElement("tool_table");
  for (const_iterator it = begin(); it != end(); it++)
    if (it->second.getNumber()) it->second.write(writer);
  writer.endElement("tool_table");
}


void ToolTable::write(JSON::Sink &sink) const {
  sink.beginDict();

  for (const_iterator it = begin(); it != end(); it++)
    if (it->second.getNumber()) {
      sink.beginInsert(String(it->second.getNumber()));
      it->second.write(sink, false);
    }

  sink.endDict();
}
