/*
 Copyright (C) 2010-2017 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>

#include "IO/PathQt.h"
#include "KeyStrings.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/Actions.h"

#include <array>
#include <filesystem>
#include <iostream>
#include <tuple>

namespace TrenchBroom::View
{

namespace
{
QString escapeString(const QString& str)
{
  return str == "'" ? "\\'" : str == "\\" ? "\\\\" : str;
}

void printKeys(QTextStream& out)
{
  const auto keyStrings = KeyStrings{};

  out << "const keys = {\n";
  for (const auto& [portable, native] : keyStrings)
  {
    out << "    '" << escapeString(portable) << "': '" << escapeString(native) << "',\n";
  }
  out << "};\n";
}

QString toString(const QStringList& path, const QString& suffix)
{
  auto result = QString{};
  result += "[";
  for (const auto& component : path)
  {
    result += "'" + component + "', ";
  }
  result += "'" + suffix + "'";
  result += "]";
  return result;
}

QString toString(const QKeySequence& keySequence)
{
  static const std::array<std::tuple<int, QString>, 4> Modifiers = {
    std::make_tuple(static_cast<int>(Qt::CTRL), QString::fromLatin1("Ctrl")),
    std::make_tuple(static_cast<int>(Qt::ALT), QString::fromLatin1("Alt")),
    std::make_tuple(static_cast<int>(Qt::SHIFT), QString::fromLatin1("Shift")),
    std::make_tuple(static_cast<int>(Qt::META), QString::fromLatin1("Meta")),
  };

  auto result = QString{};
  result += "{ ";

  if (keySequence.count() > 0)
  {
    const auto keyWithModifier = keySequence[0];
    const auto key = keyWithModifier & ~(static_cast<int>(Qt::KeyboardModifierMask));

    const auto keyPortableText = QKeySequence{key}.toString(QKeySequence::PortableText);

    result += "key: '" + escapeString(keyPortableText) + "', ";
    result += "modifiers: [";
    for (const auto& [modifier, portableText] : Modifiers)
    {
      if ((keyWithModifier & modifier) != 0)
      {
        result += "'" + escapeString(portableText) + "', ";
      }
    }
    result += "]";
  }
  else
  {
    result += "key: '', modifiers: []";
  }
  result += " }";
  return result;
}

class PrintMenuVisitor : public TrenchBroom::View::MenuVisitor
{
private:
  QTextStream& m_out;
  QStringList m_path;

public:
  explicit PrintMenuVisitor(QTextStream& out)
    : m_out{out}
  {
  }

  void visit(const Menu& menu) override
  {
    m_path.push_back(QString::fromStdString(menu.name()));
    menu.visitEntries(*this);
    m_path.pop_back();
  }

  void visit(const MenuSeparatorItem&) override {}

  void visit(const MenuActionItem& item) override
  {
    m_out << "    '" << IO::pathAsGenericQString(item.action().preferencePath()) << "': ";
    m_out << "{ path: " << toString(m_path, item.label())
          << ", shortcut: " << toString(item.action().keySequence()) << " },\n";
  }
};

void printMenuShortcuts(QTextStream& out)
{
  out << "const menu = {\n";

  const auto& actionManager = ActionManager::instance();
  auto visitor = PrintMenuVisitor{out};
  actionManager.visitMainMenu(visitor);

  out << "};\n";
}

void printActionShortcuts(QTextStream& out)
{
  out << "const actions = {\n";

  auto printPref = [&out](const auto& prefPath, const auto& keySequence) {
    out << "    '" << IO::pathAsGenericQString(prefPath) << "': ";
    out << toString(keySequence) << ",\n";
  };

  class ToolbarVisitor : public MenuVisitor
  {
  public:
    std::vector<const Action*> toolbarActions;

    void visit(const Menu& menu) override { menu.visitEntries(*this); }

    void visit(const MenuSeparatorItem&) override {}

    void visit(const MenuActionItem& item) override
    {
      const auto* tAction = &item.action();
      toolbarActions.push_back(tAction);
    }
  };

  const auto& actionManager = ActionManager::instance();
  auto visitor = ToolbarVisitor{};
  actionManager.visitToolBarActions(visitor);
  for (const auto* action : visitor.toolbarActions)
  {
    printPref(action->preferencePath(), action->keySequence());
  }
  actionManager.visitMapViewActions([&](const auto& action) {
    printPref(action.preferencePath(), action.keySequence());
  });

  // some keys are just Preferences (e.g. WASD)
  for (auto* keyPref : Preferences::keyPreferences())
  {
    printPref(keyPref->path(), keyPref->defaultValue());
  }

  out << "};\n";
}

} // namespace
} // namespace TrenchBroom::View

extern void qt_set_sequence_auto_mnemonic(bool b);

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cout << "Usage: dump-shortcuts <path-to-output-file>\n";
    return 1;
  }

  QSettings::setDefaultFormat(QSettings::IniFormat);

  // We can't use auto mnemonics in TrenchBroom. e.g. by default with Qt, Alt+D opens the
  // "Debug" menu, Alt+S activates the "Show default properties" checkbox in the entity
  // inspector. Flying with Alt held down and pressing WASD is a fundamental behaviour in
  // TB, so we can't have shortcuts randomly activating.
  qt_set_sequence_auto_mnemonic(false);

  const auto path = QString{argv[1]};
  auto file = QFile{path};
  const auto fileInfo = QFileInfo{file.fileName()};
  const auto absPath = fileInfo.absoluteFilePath().toStdString();

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  { // QIODevice::WriteOnly implies truncate, which we want
    std::cout << "Could not open output file for writing: " << absPath << "\n";
    return 1;
  }

  auto out = QTextStream{&file};
  out.setCodec("UTF-8");

  TrenchBroom::PreferenceManager::createInstance<TrenchBroom::AppPreferenceManager>();

  // QKeySequence requires that an application instance is created!
  auto app = QApplication{argc, argv};
  app.setApplicationName("TrenchBroom");
  // Needs to be "" otherwise Qt adds this to the paths returned by QStandardPaths
  // which would cause preferences to move from where they were with wx
  app.setOrganizationName("");
  app.setOrganizationDomain("io.github.trenchbroom");

  TrenchBroom::View::printKeys(out);
  TrenchBroom::View::printMenuShortcuts(out);
  TrenchBroom::View::printActionShortcuts(out);

  TrenchBroom::PreferenceManager::destroyInstance();

  out.flush();
  return out.status() == QTextStream::Ok ? 0 : 1;
}
