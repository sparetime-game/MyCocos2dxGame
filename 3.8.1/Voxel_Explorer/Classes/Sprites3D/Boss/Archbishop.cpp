//
//  Archbishop.cpp
//  Voxel_Explorer
//
//  Created by wang haibo on 15/10/22.
//
//

#include "Archbishop.hpp"
#include "LevelResourceManager.h"
#include "OutlineEffect3D.h"
#include "VoxelExplorer.h"
#include "FaneBossLevel.hpp"
#include "UtilityHelper.h"
#include "FakeShadow.hpp"
#include "ArchbishopBullet.hpp"
#include "SimpleAudioEngine.h"
USING_NS_CC;
using namespace CocosDenshion;

Archbishop* Archbishop::create(BaseBoss::BossType type)
{
    std::string model = LevelResourceManager::getInstance()->getMonsterModelRes(BOSS_MODEL_NAMES[type]);
    auto boss = new (std::nothrow) Archbishop();
    if (boss && boss->initWithFile(model) && boss->createFakeShadow())
    {
        boss->m_Type = type;
        boss->setCameraMask((unsigned int)CameraFlag::USER1);
        boss->setLightMask((unsigned int)LightFlag::LIGHT0 |(unsigned int)LightFlag::LIGHT1|(unsigned int)LightFlag::LIGHT2);
        
        OutlineEffect3D* outline = OutlineEffect3D::create();
        outline->setOutlineColor(Vec3(1.0f, 1.0f, 1.0f));
        outline->setOutlineWidth(0.03f);
        boss->addEffect(outline, 1);
        
        boss->autorelease();
        return boss;
    }
    CC_SAFE_DELETE(boss);
    return nullptr;
}
Archbishop::Archbishop()
{
    m_Type = BT_ARCHBISHOP;
    m_nFOV = 11;
    m_fSkill2Timer = 15.0f;
    m_nFleeRange = 2;
    m_bInSkill2Stage = false;
}
Archbishop::~Archbishop()
{
}
void Archbishop::update(float delta)
{
    if(!isVisible())
        return;
    switch (m_State) {
        case BS_SLEEPING:
            {
                if(isPlayerInsideBossRoom() && VoxelExplorer::getInstance()->checkBossAlert(this))
                    setState(BS_TRACKING);
            }
            break;
        case BS_WANDERING:
            {
                setState(BS_MOVING);
            }
            break;
        case BS_TRACKING:
            {
                m_fFirstTrackingTimer -= delta;
                if(m_fFirstTrackingTimer < 0)
                {
                    if(VoxelExplorer::getInstance()->checkBossCanAttack(this))
                        setState(BS_ATTACK);
                    else
                        setState(BS_MOVING);
                }
            }
            break;
        case BS_FLEEING:
            {
                if(VoxelExplorer::getInstance()->checkBossFleeFromPlayer(this))
                    setState(BS_MOVING);
                else if (VoxelExplorer::getInstance()->checkBossCanAttack(this))
                    setState(BS_ATTACK);
                else
                    setState(BS_TRACKING);
            }
            break;
        default:
            break;
    }
    if(m_bEnableFlee && m_bInSkill2Stage)
    {
        m_fSkill2Timer -= delta;
        if(m_fSkill2Timer <= 0)
        {
            FaneBossLevel* level = dynamic_cast<FaneBossLevel*>(VoxelExplorer::getInstance()->getCurrentLevel());
            if(level)
            {
                if(level->createSummoningMonstersByArchbishop(getPosInMap(), 2))
                {
                    Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_BOSS_SKILL2, this);
                }
            }
            m_fSkill2Timer = 15.0f;
        }
    }
    if(m_pFakeShadow)
        m_pFakeShadow->setPosition3D(Vec3(getPositionX(),-TerrainTile::CONTENT_SCALE*0.49f,getPositionZ()));
}
void Archbishop::onEnterTracking()
{
    if(m_LastState == BS_SLEEPING)
    {
        Director::getInstance()->getEventDispatcher()->dispatchCustomEvent(EVENT_BOSS_ALERT, this);
        m_fFirstTrackingTimer = 1;
    }
    else if(m_LastState == BS_FLEEING)
    {
        if(VoxelExplorer::getInstance()->checkBossFleeFromPlayer(this))
            setState(BS_FLEEING);
    }
}
void Archbishop::onEnterSkill1Stage()
{
    FaneBossLevel* level = dynamic_cast<FaneBossLevel*>(VoxelExplorer::getInstance()->getCurrentLevel());
    if(level)
    {
        level->createSummoningMonstersByArchbishop(getPosInMap(), 1);
    }
}
void Archbishop::onEnterSkill2Stage()
{
    m_nAttackRange = 3;
    m_bEnableFlee = true;
    m_bInSkill2Stage = true;
    setState(BS_FLEEING);
    
    if(getEffectCount() > 0)
    {
        OutlineEffect3D* outline = dynamic_cast<OutlineEffect3D*>(getEffect(0));
        if(outline)
        {
            Vec3 color = Vec3(Color3B::ORANGE.r/255.0f, Color3B::ORANGE.g/255.0f, Color3B::ORANGE.b/255.0f);
            outline->setOutlineColor(color);
        }
    }
}
void Archbishop::onEnterSkill3Stage()
{
    FaneBossLevel* level = dynamic_cast<FaneBossLevel*>(VoxelExplorer::getInstance()->getCurrentLevel());
    if(level)
    {
        level->createSummoningMonstersByArchbishop(getPosInMap(), 3);
    }
    m_nAttackRange = 3;
    m_bEnableFlee = true;
    m_bInSkill2Stage = false;
    setState(BS_FLEEING);
    
    if(getEffectCount() > 0)
    {
        OutlineEffect3D* outline = dynamic_cast<OutlineEffect3D*>(getEffect(0));
        if(outline)
        {
            Vec3 color = Vec3(Color3B::RED.r/255.0f, Color3B::RED.g/255.0f, Color3B::RED.b/255.0f);
            outline->setOutlineColor(color);
        }
    }
}
void Archbishop::onEnterDeath()
{
    if(getEffectCount() > 0)
    {
        OutlineEffect3D* outline = dynamic_cast<OutlineEffect3D*>(getEffect(0));
        if(outline)
        {
            Color3B outlineColor = UtilityHelper::randomColor();
            outline->setOutlineColor(Vec3(outlineColor.r/255.0f, outlineColor.g/255.0f, outlineColor.b/255.0f));
        }
    }
    if(m_LastPosInMap != Vec2::ZERO)
        setPosition3D(Vec3(Vec3(m_LastPosInMap.x*TerrainTile::CONTENT_SCALE, -0.5f*TerrainTile::CONTENT_SCALE, -m_LastPosInMap.y*TerrainTile::CONTENT_SCALE)));
    this->stopAllActions();
    removeTerrainTileFlag(TileInfo::ATTACKABLE);
    removeTerrainTileFlagByPos(TileInfo::ATTACKABLE, m_NextPos);
    removeTerrainTileFlagByPos(TileInfo::ATTACKABLE, m_BackPos);
    this->setVisible(false);
    if(m_pFakeShadow)
        m_pFakeShadow->setVisible(false);
    VoxelExplorer::getInstance()->addParticle3DEffect(getPosition3D(), P3D_EFFECT_TYPE::P3D_PLAYER_DEATH);
    VoxelExplorer::getInstance()->generatePickItemByBoss(getPosInMap(), (int)m_pBossProperty->getValueCopper().GetLongValue());
    
    FaneBossLevel* level = dynamic_cast<FaneBossLevel*>(VoxelExplorer::getInstance()->getCurrentLevel());
    if(level)
        level->clearBossRoom();
    
    std::string soundName = LevelResourceManager::getInstance()->getMonsterSoundEffectRes(BOSS_MODEL_NAMES[m_Type], "DEATH");
    SimpleAudioEngine::getInstance()->playEffect(soundName.c_str());
}
bool Archbishop::isPlayerInsideBossRoom()
{
    return true;
}
void Archbishop::handleAttackStyle(const cocos2d::Vec2& playerPos, const cocos2d::Vec3& dir)
{
    if(m_nAttackRange == 1)
    {
        EaseSineOut* moveUp = EaseSineOut::create(MoveTo::create(0.1f, Vec3(getPositionX(), getPositionY() + TerrainTile::CONTENT_SCALE*0.25f, getPositionZ()) + dir*0.4f));
        CallFunc* callback = CallFunc::create(CC_CALLBACK_0(VoxelExplorer::handlePlayerHurtByBoss,VoxelExplorer::getInstance(),playerPos, this));
        EaseSineOut* moveDown = EaseSineOut::create(MoveTo::create(0.1f, getPosition3D()));
        Sequence* sequenceJump = Sequence::create(moveUp, callback, moveDown, NULL);
        CallFunc* callback2 = CallFunc::create(CC_CALLBACK_0(Archbishop::onLand,this, false));
        DelayTime* delay = DelayTime::create(0.8f);
        Sequence* sequence = Sequence::create(sequenceJump, delay, callback2, NULL);
        this->runAction(Speed::create(sequence, m_pBossProperty->getFactor().GetFloatValue()));
    }
    else
    {
        EaseSineOut* moveUp = EaseSineOut::create(MoveTo::create(0.1f, Vec3(getPositionX(), getPositionY() + TerrainTile::CONTENT_SCALE*0.25f, getPositionZ()) + dir*0.4f));
        EaseSineOut* moveDown = EaseSineOut::create(MoveTo::create(0.1f, getPosition3D()));
        CallFunc* callback = CallFunc::create(CC_CALLBACK_0(Archbishop::onLand,this, false));
        DelayTime* delay = DelayTime::create(1.8f);
        Sequence* sequence = Sequence::create(moveUp, moveDown, delay, callback, NULL);
        this->runAction(Speed::create(sequence, m_pBossProperty->getFactor().GetFloatValue()));
        
        if(VoxelExplorer::getInstance()->getBulletsLayer())
        {
            ArchbishopBullet* bullet = ArchbishopBullet::create(this);
            if(bullet)
            {
                bullet->setPosition3D(getPosition3D() + Vec3(0, TerrainTile::CONTENT_SCALE*0.5f, 0));
                bullet->setBulletState(BaseBullet::BS_NORMAL);
                VoxelExplorer::getInstance()->getBulletsLayer()->addChild(bullet);
            }
        }
    }
}