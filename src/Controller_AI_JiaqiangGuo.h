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

#pragma once

#include "iController.h"
#include "iPlayer.h"
#include <vector>
#include <map>


class Controller_AI_JiaqiangGuo : public iController
{
    public: 

        Controller_AI_JiaqiangGuo();
        virtual ~Controller_AI_JiaqiangGuo() {}

        void tick(float deltaTSec);

        enum types {
            LeftTower,
            RightTower,
            KingTower,
            Mob,
        };

        enum  combatStrategyCatagory
        {
            MeleeOrRemote,
            GroundOrAir,
            AOEOrSingle,
            TargetAirOrGround,
            TroopOrIndividual
        };

        enum strategies
        {
            MeleeAttack,
            RemoteAttack,

            AttackGround,
            AttackAny,
            AttackOnlyBuilding,

            Ground,
            Air,

            AOE,
            Single,

            Troop,
            Individual,
        };

        struct mobsData
        {
            mobsData() {};
            mobsData(iPlayer::EntityData d, float score, int i, types t, std::map<combatStrategyCatagory,strategies> sts)
                :data(d),
                utilityScore(score),
                id(i),
                type(t),
                sts(sts)
            {

            };
            mobsData& operator=(const mobsData& mob) {
                mobsData newMob(mob.data, mob.utilityScore, mob.id,mob.type,mob.sts);
                return newMob;
            };
            ~mobsData() {};
            iPlayer::EntityData data;
            float utilityScore;
            int id;
            types type;
            std::map<combatStrategyCatagory, strategies> sts;
        };
        
        enum state
        {
            TargetEnemyMobs,
            Attacking,
            WaitForElixirs,
        };

        std::vector<mobsData> leftEnemyMobs;
        std::vector<mobsData> rightEnemyMobs;
        std::vector<mobsData> enemyBuildings;
        std::vector<int> alreadyAttakedMob;

        //your card pool
        std::vector<iEntityStats::MobType> buildingAttackMobs = { iEntityStats::MobType::Giant };
        std::vector<iEntityStats::MobType> remoteAttackMobs = { iEntityStats::MobType::Archer };;
        std::vector<iEntityStats::MobType> meleeAttackMobs = { iEntityStats::MobType::Swordsman };
        //AOEMobs, troops, airMobs, AOESpell and so on;

        bool placeMobStategy0(float posX, float posY, bool isNorth);
        mobsData* findPriorityMob( mobsData* mobPriority);
        Vec2 findBestPos(mobsData* enemyMob, iEntityStats::MobType theMobWantPlace);
        iPlayer::PlacementResult placeMob(Vec2 pos, bool isNorth, iEntityStats::MobType mt);
        void chooseGroundOrAir();
        void chooseAOEOrSingle();
        void chooseTargetAirOrGround();
        void chooseTroopOrIndividual();
        void chooseMeleeOrRemote(mobsData* enemymobs);
        iEntityStats::MobType combinationStrategy(std::vector<iEntityStats::MobType>currentCardPool, iEntityStats::MobType mobCombineWith, iEntityStats::MobType mt);
        int chooseCardBasedOnStrategy(mobsData* finalPriorityMob, std::vector<iEntityStats::MobType> currentCardPool,int cardIndex);
        
};