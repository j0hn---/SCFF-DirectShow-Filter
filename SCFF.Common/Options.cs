﻿// Copyright 2012 Alalf <alalf.iQLc_at_gmail.com>
//
// This file is part of SCFF-DirectShow-Filter.
//
// SCFF DSF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SCFF DSF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SCFF DSF.  If not, see <http://www.gnu.org/licenses/>.

namespace SCFF.Common {

public class Options {
  public enum Key {
    RecentFilePath1,
    RecentFilePath2,
    RecentFilePath3,
    RecentFilePath4,
    RecentFilePath5,
    FFmpegPath,
    FFmpegArguments,
    MainWindowLeft,
    MainWindowTop,
    MainWindowWidth,
    MainWindowHeight,
    MainWindowState,
    AutoApply,
    LayoutPreview,
    LayoutBorder,
    LayoutSnap,
    CompactView,
    ForceAeroOn,
    RestoreLastProfile
  }
}
}