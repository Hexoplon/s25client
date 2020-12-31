// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "AddonList.h"
#include "mygettext/mygettext.h"

/**
 *  Addon for artificially adjusting the resource supply on a map by adjusting the consumption rate of mines.
 */
class AddonIncreaseMineSupply : public AddonList
{
public:
    AddonIncreaseMineSupply()
        : AddonList(AddonId::MINE_SUPPLY, AddonGroup::Economy, _("Adjust mine supply"),
            _("Adjust how many resources are available in mines \n"
              "Affects all types of mines.\n"
              "NOTE: Enabling inexhaustible mines will effectively disable this addon."
            ),
            {
              _("Default"),
              _("50%"),
              _("33%"),
              _("25%"),
              _("100%"),
              _("300%"),
              _("-25%"),
              _("-50%"),
            })
    {}
};