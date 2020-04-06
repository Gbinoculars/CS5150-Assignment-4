// MIT License
// 
// Copyright(c) 2020 Arthur Bacon and Kevin Dill
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this softwareand associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright noticeand this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Controller_AI_JiaqiangGuo.h"

#include "Constants.h"
#include "EntityStats.h"
#include "iPlayer.h"
#include "Vec2.h"   
#include <vector>
#include <map>
#include <ctime>
#include <random>
#include <string>
#include <cmath>



const float E = 2.718f;
int updateCD;
int placeTime;

Controller_AI_JiaqiangGuo::Controller_AI_JiaqiangGuo() {
    //In the beginning, tipically the player who first place mob will under disadvantageous position.
    //So wait 100 frames, if enemy still has no mobs, the AI will attack first.
    updateCD = 100;
}


void Controller_AI_JiaqiangGuo::tick(float deltaTSec)
{   
    assert(m_pPlayer);
    
    std::vector<iEntityStats::MobType> currentCardPool = m_pPlayer->GetAvailableMobTypes();

    updateCD--;
   
    bool isNorth = m_pPlayer->isNorth();
    int enemyMobsNum = m_pPlayer->getNumOpponentMobs();
    int enemyBuildingNum = m_pPlayer->getNumOpponentBuildings();
    int ourMobNum = m_pPlayer->getNumMobs();
    float elixirs = m_pPlayer->getElixir();

    //default wait for elixirs
    overallStrategies currentState = WaitForElixirs;
    if (elixirs >= 10 && updateCD<=0) {
        currentState = currentState < Attacking ? currentState : Attacking;
    }
    if (enemyMobsNum) {
        currentState = currentState < TargetEnemyMobs ? currentState : TargetEnemyMobs;
    }
    if (ourMobNum) {
        currentState = currentState < SupportOurMobs ? currentState : SupportOurMobs;
    }

    //get info of enemybuilding
    std::map<combatStrategyCatagory, strategies> sts;
    enemyBuildings.clear();
    for (int i = 0; i < enemyBuildingNum; i++) {
        iPlayer::EntityData data = m_pPlayer->getOpponentBuilding(i);
        if (data.m_Position.Player2Game(isNorth).x == PrincessLeftX ) {
            mobsData building(data, 0.f, i, LeftTower,sts);
            enemyBuildings.push_back(building);
        }
        if (data.m_Position.Player2Game(isNorth).x == PrincessRightX) {
            mobsData building(data, 0.f, i,RightTower,sts);
            enemyBuildings.push_back(building);
        }
        if (data.m_Position.Player2Game(isNorth).x == KingX) {
            mobsData building(data, 0.f, i, KingTower,sts);
            enemyBuildings.push_back(building);
        }
    }
    std::vector<mobsData>::iterator iter;
    for (iter = enemyBuildings.begin(); iter != enemyBuildings.end();) {
        if (iter->type == LeftTower && iter->data.m_Health<=0) {
            iter = enemyBuildings.erase(iter);
            for (std::vector<mobsData>::iterator i = enemyBuildings.begin(); i != enemyBuildings.end(); i++) {
                if (i->type == KingTower) {
                    i->type = LeftTower;
                }
            }
        }
        else if (iter->type == RightTower && iter->data.m_Health <= 0) {
            iter = enemyBuildings.erase(iter);
            for (std::vector<mobsData>::iterator i = enemyBuildings.begin(); i != enemyBuildings.end(); i++) {
                if (i->type == KingTower) {
                    i->type = RightTower;
                }
            }
        }
        else iter++;
    }
    
    //choose the overall combat strategies
    switch (currentState)
    {
    //combat enemyMobs
    case TargetEnemyMobs: {
        
        //update mobs info
        leftEnemyMobs.clear();
        rightEnemyMobs.clear();
        for (int i = 0; i < enemyMobsNum; i++) {
            iPlayer::EntityData data = m_pPlayer->getOpponentMob(i);
            if (data.m_Position.Player2Game(isNorth).x < 9) {
                mobsData mobData(data, 0.f, i,Mob,sts);
                leftEnemyMobs.push_back(mobData);
            }
            else {
                mobsData mobData(data, 0.f, i,Mob,sts);
                rightEnemyMobs.push_back(mobData);
            }
        }
        
        mobsData* finalPriorityMob = NULL;
        finalPriorityMob = findPriorityMob(finalPriorityMob,isNorth);
       

        if (finalPriorityMob) { 
            float deltaY = 0.f;
            if (finalPriorityMob->data.m_Stats.getDamageType() == iEntityStats::DamageType::Melee) {
                deltaY = 6.f;
            }
            if(finalPriorityMob->data.m_Stats.getDamageType() == iEntityStats::DamageType::Ranged) {
                deltaY = 0.5f;
            }
            if (finalPriorityMob->data.m_Position.Player2Game(isNorth).y <= RIVER_TOP_Y - deltaY) {
                chooseMeleeOrRemote(finalPriorityMob);
                int cardIndex = 0;
                cardIndex = chooseCardBasedOnStrategy(finalPriorityMob, currentCardPool, cardIndex);
                iEntityStats::MobType combineMt = currentCardPool[0];
                combineMt = combinationStrategy(currentCardPool, currentCardPool[cardIndex], combineMt);
                iPlayer::PlacementResult result0 = iPlayer::PlacementResult::MobTypeUnavailable;
                iPlayer::PlacementResult result1 = iPlayer::PlacementResult::MobTypeUnavailable;
                if (combineMt != currentCardPool[cardIndex]) {
                    Vec2 pos0 = findBestPos(finalPriorityMob, combineMt,isNorth);
                    result0 = placeMob(pos0, isNorth, combineMt);
                }
                Vec2 pos1 = findBestPos(finalPriorityMob, currentCardPool[cardIndex],isNorth);
                result1 = placeMob(pos1, isNorth, currentCardPool[cardIndex]);
                if (result0 == iPlayer::PlacementResult::Success || result1 == iPlayer::PlacementResult::Success) {
                    alreadyAttakedMob.push_back(finalPriorityMob->id);
                    std::vector<iEntityStats::MobType> mts;
                    mts.push_back(currentCardPool[cardIndex]);
                    mts.push_back(combineMt);
                    speaker(finalPriorityMob, mts, isNorth);
                }
            }
        }
             
        break;
    }
    //combat enemy towewr based on the strategies preseted.
    case Attacking: {
        float max = 0;
        mobsData* finalTarget = NULL;
        std::vector<mobsData>::iterator iter;
        for (iter = enemyBuildings.begin(); iter != enemyBuildings.end();iter++) {
            if (iter->type == LeftTower || iter->type == RightTower) {
                float healthScore;
                healthScore = float(iter->data.m_Health) / float(iter->data.m_Stats.getMaxHealth());
                iter->utilityScore = healthScore;
            }
            if (iter->utilityScore >= max) {
                max = iter->utilityScore;
                finalTarget = &(*iter);
            }
        }
        if (finalTarget) {
            if (finalTarget->type == LeftTower) {
                bool success = placeMobStategy0(PrincessLeftX, RIVER_TOP_Y, isNorth);
            }
            else if (finalTarget->type == RightTower) {
                bool success = placeMobStategy0(PrincessRightX, RIVER_TOP_Y, isNorth);
            }
        }
       
        break;
    }
    //if our mobs elinimated enemyMobs how to expand advaantages
    case SupportOurMobs: {

        break;
    }
    case WaitForElixirs: {

        break;
    }
    default:
        break;
    }
}

Controller_AI_JiaqiangGuo::mobsData* Controller_AI_JiaqiangGuo::findPriorityMob(mobsData* mobPriority,bool isNorth) {
    
    if (updateCD <= 0) {
        alreadyAttakedMob.clear();
        updateCD = 100;
    }

    for (std::vector<mobsData>::iterator iter = leftEnemyMobs.begin(); iter != leftEnemyMobs.end(); iter++) {
        float healthScore;
        float positionScore;
        float attackScore;
        float x = ((iter->data.m_Position.Player2Game(isNorth).y - NorthPrincessY) -5.0f);
        healthScore = float(iter->data.m_Health) / float(iter->data.m_Stats.getMaxHealth());
        positionScore = 1.f - (1.f / (1.f + pow(E, -(x))));
        attackScore = iter->data.m_Stats.getDamage() / 1000.f;
        iter->utilityScore = healthScore * ((+positionScore + attackScore) / 2.f);
        for (int j = 0; j < alreadyAttakedMob.size(); j++) {
            if (iter->id == alreadyAttakedMob[j]) {
                iter->utilityScore = 0;
            }
        }
    }

    for (std::vector<mobsData>::iterator iter = rightEnemyMobs.begin(); iter != rightEnemyMobs.end(); iter++) {
        float healthScore;
        float positionScore;
        float attackScore;
        float x = ((iter->data.m_Position.Player2Game(isNorth).y - NorthPrincessY) - 5.0f);
        healthScore = float(iter->data.m_Health) / float(iter->data.m_Stats.getMaxHealth());
        positionScore = 1.f - (1.f / (1.f + pow(E, -(x))));
        attackScore = iter->data.m_Stats.getDamage() / 1000.f;
        iter->utilityScore = healthScore * ((positionScore + attackScore) / 2.f);
        for (int j = 0; j < alreadyAttakedMob.size(); j++) {
            if (iter->id == alreadyAttakedMob[j]) {
                iter->utilityScore = 0;
            }
        }
    }


    float max = 0;
    for (int i = 0; i < leftEnemyMobs.size(); i++) {
        if (leftEnemyMobs[i].utilityScore > max) {
            max = leftEnemyMobs[i].utilityScore;
            mobPriority = &leftEnemyMobs[i];
        }
    }

    for (int i = 0; i < rightEnemyMobs.size(); i++) {
        if (rightEnemyMobs[i].utilityScore > max) {
            max = rightEnemyMobs[i].utilityScore;
            mobPriority = &rightEnemyMobs[i];
        }
    }

    return mobPriority;
}

Vec2 Controller_AI_JiaqiangGuo::findBestPos(mobsData* enemyMob,iEntityStats::MobType theMobWantPlace,bool isNorth) {
    if (iEntityStats::getStats(theMobWantPlace).getDamageType() == iEntityStats::DamageType::Melee) {
        return enemyMob->data.m_Position.Player2Game(isNorth);
    }
    
    if (iEntityStats::getStats(theMobWantPlace).getDamageType() == iEntityStats::DamageType::Ranged) {
        if (enemyMob->data.m_Position.Player2Game(isNorth).y - 6.5f > 0.0f) {
            Vec2 pos(enemyMob->data.m_Position.Player2Game(isNorth).x, enemyMob->data.m_Position.Player2Game(isNorth).y - 6.5f);
            return pos;
        }
        else {
            float daltaY = abs(enemyMob->data.m_Position.Player2Game(isNorth).y - 7.0f);
            if (enemyMob->data.m_Position.Player2Game(isNorth).x - daltaY > 0) {
                Vec2 pos(enemyMob->data.m_Position.Player2Game(isNorth).x - daltaY, enemyMob->data.m_Position.Player2Game(isNorth).y - 7.0f + daltaY);
                return pos;
            }
            else {
                Vec2 pos(enemyMob->data.m_Position.Player2Game(isNorth).x + daltaY, enemyMob->data.m_Position.Player2Game(isNorth).y - 7.0f + daltaY);
                return pos;
            }
        }
    }

}

iPlayer::PlacementResult Controller_AI_JiaqiangGuo::placeMob(Vec2 pos, bool isNorth, iEntityStats::MobType mt) {
    Vec2 pos_Game = pos.Player2Game(isNorth);
    
    return m_pPlayer->placeMob(mt, pos_Game);
}


bool Controller_AI_JiaqiangGuo::placeMobStategy0(float posX, float posY, bool isNorth) {
    Vec2 pos(posX, posY - 0.5f);
    iPlayer::PlacementResult result0 = placeMob(pos, isNorth, bigCardPool[0]);
    pos.x = posX;
    pos.y = posY - 4.5f; 
    iPlayer::PlacementResult result1 = placeMob(pos, isNorth, bigCardPool[2]);
    pos.x = posX;
    pos.y = posY - 7.5f;
    iPlayer::PlacementResult result2 = placeMob(pos, isNorth, bigCardPool[1]);
    std::vector <iEntityStats::MobType> mts;
    mts.push_back(bigCardPool[0]);
    mts.push_back(bigCardPool[2]);
    mts.push_back(bigCardPool[1]);
    speaker(NULL, mts, isNorth);
    if (result0 == 0 && result1 == 0 && result2 == 0) {
        return true;
    }
    else return false;
}

int Controller_AI_JiaqiangGuo::chooseCardBasedOnStrategy(mobsData* finalPriorityMob, std::vector<iEntityStats::MobType> currentCardPool ,int cardIndex) {

    std::map<combatStrategyCatagory, strategies>::iterator iter;
    std::vector<int> cardScore(4, 0);
    for (iter = finalPriorityMob->sts.begin(); iter != finalPriorityMob->sts.end(); iter++) {
        //the first priority is TargetAirOrGround
        //because use a mob which target ground unit to combat with a enemy mob which is an air unit is stupid
        //the second priority is TroopOrIndividual
        //the third priority is AOEOrSingle
        //the fourth and fifth are  MeleeOrRemote GroundOrAir
        if (iter->first == MeleeOrRemote) {
            for (int i = 0; i < currentCardPool.size(); i++) {
                if (iter->second == MeleeAttack) {
                    if (iEntityStats::getStats(currentCardPool[i]).getDamageType() == iEntityStats::DamageType::Melee 
                        && iEntityStats::getStats(currentCardPool[i]).getTargetType() != iEntityStats::TargetType::Building) {
                        cardScore[i] ++;
                    }
                }
                else if (iter->second == RemoteAttack) {
                    if (iEntityStats::getStats(currentCardPool[i]).getDamageType() == iEntityStats::DamageType::Ranged 
                        && iEntityStats::getStats(currentCardPool[i]).getTargetType() != iEntityStats::TargetType::Building) {
                        cardScore[i] ++;
                    }
                }
            }
        }
        

        int max = 0;
        for (int i = 0; i < cardScore.size(); i++)
        {
            if (cardScore[i] > max) {
                max = cardScore[i];
                cardIndex = i;
            }
        }
        return cardIndex;
    }
}

iEntityStats::MobType Controller_AI_JiaqiangGuo::combinationStrategy(std::vector<iEntityStats::MobType>currentCardPool, iEntityStats::MobType mobCombineWith, iEntityStats::MobType mt) {
    if (iEntityStats::getStats(mobCombineWith).getDamageType() == iEntityStats::DamageType::Ranged) {
        for (iEntityStats::MobType mob : currentCardPool) {
            if (iEntityStats::getStats(mob).getDamageType() == iEntityStats::DamageType::Melee && iEntityStats::getStats(mob).getTargetType() != iEntityStats::TargetType::Building) {
                mt = mob;
                return mt;
            }
        }
    }
    else return mobCombineWith;
}

void Controller_AI_JiaqiangGuo::speaker(mobsData* finalPriorityMob, std::vector<iEntityStats::MobType> theMobsWillPlace,bool isNorth) {
    std::cout << "*********************************************************************************************************************************\n";
    std::string myPlace = "";
    std::string enemyPlace = "";
    if (isNorth) {
        myPlace = "Northern AI";
        enemyPlace = "Southern AI";
    }
    else {
        myPlace = "Southern AI";
        enemyPlace = "Northern AI";
    }
    if (finalPriorityMob) {
        std::cout << "Now the priority target is " << enemyPlace << "'s " << finalPriorityMob->data.m_Stats.getName()<<".\n";
        std::cout << myPlace << " wants to use ";
        for (int i = 0; i < theMobsWillPlace.size(); i++) {
            if (i == 0)
            {
                std::cout << iEntityStats::getStats(theMobsWillPlace[i]).getName() << " combining with ";
            }
            else if (i != theMobsWillPlace.size() - 1 ) {
                std::cout << iEntityStats::getStats(theMobsWillPlace[i]).getName() << " and ";
            }
            else {
                std::cout << iEntityStats::getStats(theMobsWillPlace[i]).getName();
            }
        }


        std::cout << " to combat with " << enemyPlace << "'s " << finalPriorityMob->data.m_Stats.getName() << ".\n";
        std::cout << "The " << myPlace << " think this strategies can take the adavantages.\n";
        std::cout << "Because";
        for (std::map<combatStrategyCatagory, strategies>::iterator iter = finalPriorityMob->sts.begin(); iter != finalPriorityMob->sts.end(); iter++) {
            if (iter->second == strategies::MeleeAttack) {
                std::cout << " melee attack overcome the ranged in short distance.\n";
            }
            else if (iter->second == strategies::RemoteAttack) {
                std::cout << " remote attack overcome the melee in long distance and best combine with the melee.\n";
            }
        }
    }
    else {
        std::cout << "Now the priority Target is " << enemyPlace << "'s tower." <<"\n";
        std::cout << myPlace << " thinks the best strategy to attack " << enemyPlace << " is to use the combination of ";
        for (int i = 0; i < theMobsWillPlace.size(); i++) {
            if (i != theMobsWillPlace.size() - 1) {
                std::cout << iEntityStats::getStats(theMobsWillPlace[i]).getName() << " and ";
            }
            else {
                std::cout << iEntityStats::getStats(theMobsWillPlace[i]).getName()<<".\n";
            }
        }
        std::cout << "Bacause this is a good way to attack the tower and take the advantages.\n";
    }
    std::cout << "*********************************************************************************************************************************\n";

}

void Controller_AI_JiaqiangGuo::chooseMeleeOrRemote(mobsData* enemyMob) {
    if (enemyMob->data.m_Stats.getDamageType() == iEntityStats::DamageType::Melee) {
        enemyMob->sts[MeleeOrRemote] = RemoteAttack;
    }
    else if (enemyMob->data.m_Stats.getDamageType() == iEntityStats::DamageType::Ranged) {
        enemyMob->sts[MeleeOrRemote] = MeleeAttack;
    }
}

//becuase now there are no other tpyes of mobs ...
//void Controller_AI_JiaqiangGuo::chooseAOEOrSingle() {}
//void Controller_AI_JiaqiangGuo::chooseTargetAirOrGround() {}
//...

