// Copyright (c) 2005 - 2020 Settlers Freaks (sf-team at siedler25.org)
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

#include "nofMiner.h"
#include "GlobalGameSettings.h"
#include "Loader.h"
#include "SoundManager.h"
#include "addons/const_addons.h"
#include "buildings/nobUsual.h"
#include "network/GameClient.h"
#include "ogl/glArchivItem_Bitmap_Player.h"
#include "world/GameWorldGame.h"
#include "random/Random.h"

nofMiner::nofMiner(const MapPoint pos, const unsigned char player, nobUsual* workplace)
    : nofWorkman(JOB_MINER, pos, player, workplace)
{}

nofMiner::nofMiner(SerializedGameData& sgd, const unsigned obj_id) : nofWorkman(sgd, obj_id) {}

void nofMiner::DrawWorking(DrawPoint drawPt)
{
    const helpers::MultiArray<DrawPoint, NUM_NATIONS, 4>
      offsets = // work animation offset per nation and (granite, coal, iron, gold)
      {{
        {{5, 3}, {5, 3}, {5, 3}, {5, 3}},     // africans
        {{4, 1}, {4, 1}, {4, 1}, {4, 1}},     // japanese
        {{9, 4}, {9, 4}, {9, 4}, {9, 4}},     // romans
        {{10, 3}, {10, 3}, {10, 3}, {10, 3}}, // vikings
        {{8, 3}, {8, 3}, {8, 3}, {8, 3}}      // babylonians
      }};

    unsigned now_id = GAMECLIENT.Interpolate(160, current_ev);
    unsigned texture;
    if(workplace->GetNation() == NAT_ROMANS)
        texture = 92 + now_id % 8;
    else
        texture = 1799 + now_id % 4;
    LOADER.GetPlayerImage("rom_bobs", texture)
      ->DrawFull(drawPt + offsets[workplace->GetNation()][workplace->GetBuildingType() - BLD_GRANITEMINE]);

    if(now_id % 8 == 3)
    {
        SOUNDMANAGER.PlayNOSound(59, this, now_id);
        was_sounding = true;
    }
}

unsigned short nofMiner::GetCarryID() const
{
    switch(workplace->GetBuildingType())
    {
        case BLD_GOLDMINE: return 65;
        case BLD_IRONMINE: return 66;
        case BLD_COALMINE: return 67;
        default: return 68;
    }
}

helpers::OptionalEnum<GoodType> nofMiner::ProduceWare()
{
    switch(workplace->GetBuildingType())
    {
        case BLD_GOLDMINE: return GD_GOLD;
        case BLD_IRONMINE: return GD_IRONORE;
        case BLD_COALMINE: return GD_COAL;
        default: return GD_STONES;
    }
}

bool nofMiner::AreWaresAvailable() const
{
    return nofWorkman::AreWaresAvailable() && FindPointWithResource(GetRequiredResType()).isValid();
}

bool nofMiner::StartWorking()
{
    MapPoint resPt = FindPointWithResource(GetRequiredResType());
    if(!resPt.isValid())
        return false;
    const GlobalGameSettings& settings = gwg->GetGGS();
    bool inexhaustibleRes =
      settings.isEnabled(AddonId::INEXHAUSTIBLE_MINES)
      || (workplace->GetBuildingType() == BLD_GRANITEMINE && settings.isEnabled(AddonId::INEXHAUSTIBLE_GRANITEMINES));
    if (!inexhaustibleRes){
        // Numerator and denomenator indicating how often to not consume resources. Numerators are zero indexed for the first three entries, so 0/3 -> 1/3
        // For the last two sets of numerators and denomenators.
        // When the supply is increased, the probability of not consuming resources is applied to the same resource over multiple iterations, until they
        // are finally gone. Assuming 1000 resources on a point, after one itteration with the 50% increase option, 333 are remaining. These are then mined
        // using the same probabilites, leaving 111 resources and so on. The sum of all the available show a 50% increase with a 1/3 chance of not consuming resources.
        // The same logic does not apply when reducing the amount of resources, as they will not pass through twice, hence why decreasing supply uses normal fractions
        // to represent the decrease in supply.

        // The numbers mean:
        // 0. Default, consume at normal rate
        // 1. skip consumption 1/3 of the time gives 50% total increase in resources
        // 2. skip consumption 1/4 of the time, gives 33% total increase in resources
        // 3. skip consumption 1/5 of the time, gives 25% total increase in resources
        // 4. skip consumption 2/3 of the time, gives 100% total increase in resources
        // 5. skip consumption 3/4 of the time, gives 300% total increase in resources
        // 6. 1/4 of the time, consume one extra resource, giving a 25% reduction in resources
        // 7. 1/2 of the time, consume one extra resource, giving a 50% reduction in resources
        std::array<int, 8> numerator = { 0, 0, 0, 0, 2, 3 , 0, 0};
        std::array<int, 8> denomenator = { 0, 3, 4, 5, 3, 4, 4, 2 };
        unsigned selection = settings.getSelection(AddonId::MINE_SUPPLY);
        
        //If MINE_SUPPLY is not enabled, the mines will conume resources like normally.

        switch (selection)
        {
            case 0:
            default: gwg->ReduceResource(resPt); break; //Normal operation, or player chose an increase less than 100%
            case 1:
            case 2:
            case 3: // Increase mine supply by less than 100%
                if (RANDOM.Rand(__FILE__, __LINE__, GetObjId(), denomenator[selection]) > numerator[selection])
                    gwg->ReduceResource(resPt);
                break;
            case 4:
            case 5: // Increase mine supply by 100% or more
                if (RANDOM.Rand(__FILE__, __LINE__, GetObjId(), denomenator[selection]) < numerator[selection])
                    gwg->ReduceResource(resPt);
                break;
            case 6:
            case 7: // Decrease mine supply by consuming at normal rate pluss sometimes more. Only perform double consumption if there are two or more resources available in the given point.
                gwg->ReduceResource(resPt);
                if (RANDOM.Rand(__FILE__, __LINE__, GetObjId(), denomenator[selection]) > numerator[selection] && gwg->GetNode(resPt).resources.getAmount() > 1)
                    gwg->ReduceResource(resPt);
                break;
        }
    }
    return nofWorkman::StartWorking();
}

Resource::Type nofMiner::GetRequiredResType() const
{
    switch(workplace->GetBuildingType())
    {
        case BLD_GOLDMINE: return Resource::Gold;
        case BLD_IRONMINE: return Resource::Iron;
        case BLD_COALMINE: return Resource::Coal;
        default: return Resource::Granite;
    }
}
