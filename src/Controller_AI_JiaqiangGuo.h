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

    protected:

        //the overall strategies
        enum overallStrategies
        {
            TargetEnemyMobs,
            SupportOurMobs,
            Attacking,
            WaitForElixirs,
        };

        //the types of units
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
            TroopOrIndividual,
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

        //struct contain all info of the units
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

            //this map cantain what strategy suitable to combat with this unit
            std::map<combatStrategyCatagory, strategies> sts;
        };

        
        std::vector<mobsData> leftEnemyMobs;
        std::vector<mobsData> rightEnemyMobs;
        std::vector<mobsData> enemyBuildings;
        std::vector<int> alreadyAttakedMob;
        

        // card pool
        std::vector<iEntityStats::MobType> bigCardPool = { iEntityStats::MobType::Giant , iEntityStats::MobType::Swordsman, iEntityStats::Archer};

        //preset strategies
        bool placeMobStategy0(float posX, float posY, bool isNorth);

        //find the mob AI need to deal with first
        mobsData* findPriorityMob( mobsData* mobPriority, bool isNorth);

        //find the position on that our mob can take the most advantages
        Vec2 findBestPos(mobsData* enemyMob, iEntityStats::MobType theMobWantPlace, bool isNorth);

        iPlayer::PlacementResult placeMob(Vec2 pos, bool isNorth, iEntityStats::MobType mt);

        //calculate different strategies(see .h emun strategies) describe that the AI use which types of mob to attack enemyMob
        void chooseGroundOrAir();
        void chooseAOEOrSingle();
        void chooseTargetAirOrGround();
        void chooseTroopOrIndividual();
        void chooseMeleeOrRemote(mobsData* enemymobs);

        //find the mob from card pool that is most suitable to attack the most prioritized enemy mob based on strategies(see .h emun strategies). 
        int chooseCardBasedOnStrategy(mobsData* finalPriorityMob, std::vector<iEntityStats::MobType> currentCardPool,int cardIndex);

        //the mob AI want to place need which types of mob to support, for example the archers need an tank help them take the damage.
        iEntityStats::MobType combinationStrategy(std::vector<iEntityStats::MobType>currentCardPool, iEntityStats::MobType mobCombineWith, iEntityStats::MobType mt);

        //explain what happend and why this decision was made.
        void speaker(mobsData* finalPriorityMob, std::vector<iEntityStats::MobType> theMobsYouPlaced, bool isNorth);
        
};