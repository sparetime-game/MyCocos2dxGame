//
//  Area.cpp
//  Voxel_Explorer
//
//  Created by wang haibo on 15/9/24.
//
//

#include "Area.h"
#include "BaseLevel.h"
#include "AlisaMethod.h"
USING_NS_CC;

Area::AREA_TYPE AT_SpecialTypes[] = { Area::AREA_TYPE::AT_SPECIAL_EQUIPMENT_SHOP, Area::AREA_TYPE::AT_SPECIAL_MAGIC_SHOP, Area::AREA_TYPE::AT_SPECIAL_AID_STATION, Area::AREA_TYPE::AT_SPECIAL_ALCHEMIST_ROOM, Area::AREA_TYPE::AT_SPECIAL_WITCH_ROOM, Area::AREA_TYPE::AT_SPECIAL_THEIF_ROOM, Area::AREA_TYPE::AT_SPECIAL_SAGE_ROOM, Area::AREA_TYPE::AT_SPECIAL_MISSION_ROOM, Area::AREA_TYPE::AT_SPECIAL_TREASURE_ROOM, Area::AREA_TYPE::AT_SPECIAL_DECORATION_ROOM, Area::AREA_TYPE::AT_SPECIAL_TRANSPOT_ROOM };
std::vector<Area::AREA_TYPE> Area::SPECIALS(AT_SpecialTypes, AT_SpecialTypes+ sizeof(AT_SpecialTypes)/sizeof(Area::AREA_TYPE));
Area* Area::create()
{
    Area *pRet = new(std::nothrow) Area();
    if (pRet && pRet->init())
    {
        pRet->autorelease();
        return pRet;
    }
    else
    {
        CC_SAFE_DELETE(pRet);
        return nullptr;
    }
}
Area::Area()
{
    m_Type = AT_UNKNOWN;
    m_nWeight = 1;
}
Area::~Area()
{
}
bool Area::init()
{
    return true;
}

void Area::addNeigbourArea( Area* other )
{
    if(!other)
        return;
    cocos2d::Rect rect = getIntersectRect(other);
    if((rect.size.width == 0 && rect.size.height>=3) || (rect.size.height == 0 && rect.size.width >= 3))
    {
        m_Neigbours.push_back(other);
        other->m_Neigbours.push_back(this);
    }
}
void Area::connectArea(Area* other)
{
    if(!other)
        return;
    if (m_ConnectedAreas.find(other) == m_ConnectedAreas.end()) {
        m_ConnectedAreas[other] = nullptr;
        other->m_ConnectedAreas[this] = nullptr;
    }
}
Door* Area::getEntrance()
{
    if(m_ConnectedAreas.empty())
        return nullptr;
    return (*m_ConnectedAreas.begin()).second;
}

void Area::generate(BaseLevel* level)
{
    switch (m_Type) {
        case AT_STANDARD:
            generateStandardArea(level);
            break;
        case AT_ENTRANCE:
            generateEntranceArea(level);
            break;
        case AT_EXIT:
            generateExitArea(level);
            break;
        case AT_BOSS_ROOM:
            generateBossRoomArea(level);
            break;
        case AT_BOSS_EXIT:
            generateBossExitArea(level);
            break;
        case AT_PASSAGE:
            generatePassageArea(level);
            break;
        case AT_TUNNEL:
            generateTunnelArea(level);
            break;
        case AT_SPECIAL_EQUIPMENT_SHOP:
        case AT_SPECIAL_MAGIC_SHOP:
        case AT_SPECIAL_AID_STATION:
        case AT_SPECIAL_ALCHEMIST_ROOM:
        case AT_SPECIAL_MISSION_ROOM:
            generateSpecialArea(level);
            break;
        case AT_SPECIAL_WITCH_ROOM:
        case AT_SPECIAL_THEIF_ROOM:
        case AT_SPECIAL_SAGE_ROOM:
            generateSpecialHiddenArea(level);
            break;
        case AT_SPECIAL_TREASURE_ROOM:
        case AT_SPECIAL_DECORATION_ROOM:
            generateSpecialLockedArea(level);
            break;
        case AT_SPECIAL_TRANSPOT_ROOM:
            generateSpecialTranspotArea(level);
            break;
        default:
            break;
    }
}

Area::AREA_TYPE Area::getAreaType() const
{
    return m_Type;
}
void Area::setAreaType(Area::AREA_TYPE type)
{
    if(m_Type != type)
        m_Type = type;
}

cocos2d::Rect Area::getRect() const
{
    return m_Rect;
}
void Area::setRect(const cocos2d::Rect& rect)
{
    m_Rect = rect;
}
Vec2 Area::getCenter()
{
    return Vec2( (int)(m_Rect.getMinX() + m_Rect.getMaxX())/2 + (((int)(m_Rect.getMaxX()-m_Rect.getMinX()) & 1) == 1 ? cocos2d::random(0, 1) : 0), (int)(m_Rect.getMinY() + m_Rect.getMaxY())/2 + (((int)(m_Rect.getMaxY()-m_Rect.getMinY()) & 1) == 1 ? cocos2d::random(0, 1) : 0));
}
bool Area::checkInside(int p, BaseLevel* level)
{
    int x = p % level->getWidth();
    int y = p / level->getWidth();
    return checkInside(Vec2(x, y));
}
bool Area::checkInside(const cocos2d::Vec2& pos)
{
    return pos.x >= m_Rect.getMinX() && pos.y >= m_Rect.getMinY() && pos.x <= m_Rect.getMaxX() && pos.y <= m_Rect.getMaxY();
}
bool Area::isHiddenArea()
{
    bool hiddenArea = false;
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door && (door->getDoorType() == Door::DT_HIDDEN || door->getDoorType() == Door::DT_LOCKED))
            return true;
    }
    return hiddenArea;
}
cocos2d::Rect Area::getIntersectRect(Area* other)
{
    CCASSERT(other != nullptr, "Other area must not be null!");
    cocos2d::Rect rect;
    int minX = std::max(m_Rect.getMinX(), other->getRect().getMinX());
    int minY = std::max(m_Rect.getMinY(), other->getRect().getMinY());
    int maxX = std::min(m_Rect.getMaxX(), other->getRect().getMaxX());
    int maxY = std::min(m_Rect.getMaxY(), other->getRect().getMaxY());
    rect.origin.x = minX;
    rect.origin.y = minY;
    rect.size.width = maxX - minX;
    rect.size.height = maxY - minY;
    return rect;
}

int Area::getRandomTile(BaseLevel* level)
{
    return getRandomTile(level, 1);
}
int Area::getRandomTile(BaseLevel* level, int m)
{
    int left = m_Rect.getMinX() + m;
    int right = m_Rect.getMaxX() - m;
    int bottom = m_Rect.getMinY() + m;
    int top = m_Rect.getMaxY() - m;
    int x = cocos2d::random(MIN(left, right), MAX(left, right));
    int y = cocos2d::random(MIN(bottom, top), MAX(bottom, top));
    
    return x + y * level->getWidth();
}
std::vector<int> Area::getTilesOnEdge(BaseLevel* level, int m)
{
    int leftOuter = m_Rect.getMinX() + 1;
    int rightOuter = m_Rect.getMaxX() - 1;
    int bottomOuter = m_Rect.getMinY() + 1;
    int topOuter = m_Rect.getMaxY() - 1;
    
    int leftInner = m_Rect.getMinX() + m + 1;
    int rightInner = m_Rect.getMaxX() - m - 1;
    int bottomInner = m_Rect.getMinY() + m + 1;
    int topInner = m_Rect.getMaxY() - m - 1;
    cocos2d::Rect rectOuter = cocos2d::Rect(MIN(leftOuter, rightOuter), MIN(bottomOuter, topOuter), std::abs(rightOuter - leftOuter), std::abs(topOuter - bottomOuter));
    
    cocos2d::Rect rectInner = cocos2d::Rect(MIN(leftInner, rightInner), MIN(bottomInner, topInner), std::abs(rightInner - leftInner), std::abs(topInner - bottomInner));
    
    std::vector<int> ret;
    for (int j = MIN(bottomOuter, topOuter); j <= MAX(bottomOuter, topOuter); ++j)
        for (int i = MIN(leftOuter, rightOuter); i <= MAX(leftOuter, rightOuter); ++i)
    {
        if(rectInner.containsPoint(Vec2(i, j)))
            continue;
        int index = i + j*level->getWidth();
        ret.push_back(index);
    }
    return ret;
}
std::vector<int> Area::getTilesOnCorner(BaseLevel* level)
{
    int left = m_Rect.getMinX() + 1;
    int right = m_Rect.getMaxX() - 1;
    int bottom = m_Rect.getMinY() + 1;
    int top = m_Rect.getMaxY() - 1;
    
    std::vector<int> ret;
    int leftTop = left + top * level->getWidth();
    ret.push_back(leftTop);
    int rightTop = right + top * level->getWidth();
    ret.push_back(rightTop);
    int leftBottom = left + bottom * level->getWidth();
    ret.push_back(leftBottom);
    int rightBottom = right + bottom * level->getWidth();
    ret.push_back(rightBottom);
    
    return ret;
}
void Area::updateAreaFogOfWar(BaseLevel* level, bool visited)
{
    if(!level)
        return;
    int x = m_Rect.origin.x;
    int y = m_Rect.origin.y;
    int w = m_Rect.size.width+1;
    int h = m_Rect.size.height+1;
    
    level->updateTerrainTileFogOfWar(x, y, w, h, visited);
}
void Area::generateStandardArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        Area* other = iter->first;
        if(door && other)
        {
            door->setDoorType(Door::DT_STANDARD);
            if(other->getAreaType() != Area::AT_BOSS_ROOM)
            {
                float percentHidden = 0.1f;
                float percentStandard = 1.0 - percentHidden;
                AlisaMethod* am = AlisaMethod::create(percentHidden, percentStandard,-1.0, NULL);
                if(am)
                {
                    if(am->getRandomIndex() == 0)
                        door->setDoorType(Door::DT_HIDDEN);
                }
            }
            else
                door->setDoorType(Door::DT_ARENA);
        }
    }
}
void Area::generateEntranceArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_STANDARD);
    }
    setRandomTerrainTile(level, 2, TerrainTile::TT_ENTRANCE);
}
void Area::generateExitArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_STANDARD);
    }
    setRandomTerrainTile(level, 2, TerrainTile::TT_EXIT);
}
void Area::generateBossRoomArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Area* area = iter->first;
        if(!area)
            continue;
        Door* door = iter->second;
        if(door)
        {
            if(area->getAreaType() == Area::AT_BOSS_EXIT)
                door->setDoorType(Door::DT_LOCKED_EXIT);
            else
                door->setDoorType(Door::DT_ARENA);
        }
    }
}
void Area::generateBossExitArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_LOCKED_EXIT);
    }
    Vec2 center = getCenter();
    if(m_Rect.size.width >= m_Rect.size.height)
    {
        setTerrainTile(level, center.x - 1, center.y, TerrainTile::TT_STANDARD_PORTAL);
        setTerrainTile(level, center.x + 1, center.y, TerrainTile::TT_STANDARD_PORTAL);
    }
    else
    {
        setTerrainTile(level, center.x, center.y - 1, TerrainTile::TT_STANDARD_PORTAL);
        setTerrainTile(level, center.x, center.y + 1, TerrainTile::TT_STANDARD_PORTAL);
    }
}
void Area::generateTunnelArea(BaseLevel* level)
{
    Vec2 c = getCenter();
    
    if (m_Rect.size.width > m_Rect.size.height || (m_Rect.size.width == m_Rect.size.height && cocos2d::random(0, 1) == 0)) {
        
        int from = m_Rect.getMaxX() - 1;
        int to = m_Rect.getMinX() + 1;
        
        for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
            Door* door = iter->second;
            if(door)
            {
                int step = door->getPos().y < c.y ? +1 : -1;
                
                if (door->getPos().x == m_Rect.getMinX()) {
                    
                    from =  m_Rect.getMinX() + 1;
                    for (int i=door->getPos().y; i != c.y; i += step) {
                        setTerrainTile( level, from, i, TerrainTile::TT_TUNNEL );
                    }
                    
                } else if (door->getPos().x == m_Rect.getMaxX()) {
                    
                    to = m_Rect.getMaxX() - 1;
                    for (int i=door->getPos().y; i != c.y; i += step) {
                        setTerrainTile( level, to, i, TerrainTile::TT_TUNNEL );
                    }
                    
                } else {
                    if (door->getPos().x < from) {
                        from = door->getPos().x;
                    }
                    if (door->getPos().x > to) {
                        to = door->getPos().x;
                    }
                    
                    for (int i=door->getPos().y+step; i != c.y; i += step) {
                        setTerrainTile( level, door->getPos().x, i, TerrainTile::TT_TUNNEL );
                    }
                }

            }
        }
        
        for (int i=from; i <= to; i++) {
            setTerrainTile( level, i, c.y, TerrainTile::TT_TUNNEL );
        }
        
    } else {
        
        int from = m_Rect.getMaxY() - 1;
        int to = m_Rect.getMinY() + 1;
        for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
            Door* door = iter->second;
            if(door)
            {
                int step = door->getPos().x < c.x ? +1 : -1;
                
                if (door->getPos().y == m_Rect.getMinY()) {
                    
                    from = m_Rect.getMinY() + 1;
                    for (int i=door->getPos().x; i != c.x; i += step) {
                        setTerrainTile( level, i, from, TerrainTile::TT_TUNNEL );
                    }
                    
                } else if (door->getPos().y == m_Rect.getMaxY()) {
                    
                    to = m_Rect.getMaxY() - 1;
                    for (int i=door->getPos().x; i != c.x; i += step) {
                        setTerrainTile( level, i, to, TerrainTile::TT_TUNNEL );
                    }
                    
                } else {
                    if (door->getPos().y < from) {
                        from = door->getPos().y;
                    }
                    if (door->getPos().y > to) {
                        to = door->getPos().y;
                    }
                    
                    for (int i=door->getPos().x+step; i != c.x; i += step) {
                        setTerrainTile( level, i, door->getPos().y, TerrainTile::TT_TUNNEL );
                    }
                }

            }
        }
        
        for (int i=from; i <= to; i++) {
            setTerrainTile( level, c.x, i, TerrainTile::TT_TUNNEL );
        }
    }
    
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        Area* other = iter->first;
        if(door && other)
        {
            if(door->getDoorType() != Door::DT_EMPTY)
                continue;
            if(other->getAreaType() != Area::AT_BOSS_ROOM)
                door->setDoorType(Door::DT_TUNNEL);
            else
                door->setDoorType(Door::DT_ARENA);
        }
    }
}
void Area::generatePassageArea(BaseLevel* level)
{
    int pasWidth = m_Rect.size.width - 2;
    int pasHeight = m_Rect.size.height - 2;
    std::vector<int> joints;
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            joints.push_back(xy2p(m_Rect, door->getPos()));
    }
    std::sort(joints.begin(), joints.end());
    
    int nJoints = (int)joints.size();
    int perimeter = pasWidth * 2 + pasHeight * 2;
    
    int start = 0;
    int maxD = joints[0] + perimeter - joints[nJoints - 1];
    for (int i=1; i < nJoints; i++) {
        int d = joints[i] - joints[i - 1];
        if (d > maxD) {
            maxD = d;
            start = i;
        }
    }
    
    int end = (start + nJoints - 1) % nJoints;
    
    int p = joints[start];
    do {
        Vec2  xy= p2xy( m_Rect, p );
        setTerrainTile( level, xy.x, xy.y, TerrainTile::TT_STANDARD );
        p = (p + 1) % perimeter;
    } while (p != joints[end]);
    
    Vec2  xy= p2xy( m_Rect, p );
    setTerrainTile( level, xy.x, xy.y, TerrainTile::TT_STANDARD );
    
    int x = m_Rect.origin.x;
    int y = m_Rect.origin.y;
    int w = m_Rect.size.width+1;
    int h = m_Rect.size.height+1;
    int pos = y * level->getWidth() + x;
    for (int i = y; i < y + h; i++, pos += level->getWidth()) {
        for (int j = pos; j<(pos + w); j++) {
            
        }
    }
    wrapTerrainTiles(level, TerrainTile::TT_STANDARD, TerrainTile::TT_WALL);
    
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        Area* other = iter->first;
        if(door && other)
        {
            if(door->getDoorType() != Door::DT_EMPTY)
                continue;
            if(other->getAreaType() != Area::AT_BOSS_ROOM)
                door->setDoorType(Door::DT_PASSAGE);
            else
                door->setDoorType(Door::DT_ARENA);
        }
    }
}
void Area::generateSpecialArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_STANDARD);
    }
}
void Area::generateSpecialHiddenArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_HIDDEN);
    }
}
void Area::generateSpecialLockedArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_LOCKED);
    }
}
void Area::generateSpecialTranspotArea(BaseLevel* level)
{
    generateTerrainTiles(level, TerrainTile::TT_WALL);
    generateTerrainTiles(level, 1, TerrainTile::TT_STANDARD);
    for (auto iter = m_ConnectedAreas.begin(); iter != m_ConnectedAreas.end(); iter++) {
        Door* door = iter->second;
        if(door)
            door->setDoorType(Door::DT_HIDDEN);
    }
    Vec2 center = getCenter();
    setTerrainTile(level, center.x, center.y, TerrainTile::TT_SMALL_PORTAL);
}
void Area::generateTerrainTiles(BaseLevel* level, TerrainTile::TileType type)
{
    if(!level)
        return;
    int x = m_Rect.origin.x;
    int y = m_Rect.origin.y;
    int w = m_Rect.size.width+1;
    int h = m_Rect.size.height+1;
    
    level->generateTerrainTiles(x, y, w, h, type, m_Type);
}
void Area::generateTerrainTiles(BaseLevel* level, int m, TerrainTile::TileType type)
{
    if(!level)
        return;
    int x = m_Rect.origin.x + m;
    int y = m_Rect.origin.y + m;
    int w = m_Rect.size.width+1 - m*2;
    int h = m_Rect.size.height+1 - m*2;
    
    level->generateTerrainTiles(x, y, w, h, type, m_Type);
}
void Area::generateTerrainTiles(BaseLevel* level, cocos2d::Rect rect, TerrainTile::TileType type)
{
    if(!level)
        return;
    int x = m_Rect.origin.x + rect.origin.x;
    int y = m_Rect.origin.y + rect.origin.y;
    int w = m_Rect.size.width+1 - (rect.getMaxX() + rect.getMinX());
    int h = m_Rect.size.height+1 - (rect.getMaxY() + rect.getMinY());
    
    level->generateTerrainTiles(x, y, w, h, type, m_Type);
}
void Area::wrapTerrainTiles(BaseLevel* level, TerrainTile::TileType type, TerrainTile::TileType withType)
{
    if(!level)
        return;
    int x = m_Rect.origin.x;
    int y = m_Rect.origin.y;
    int w = m_Rect.size.width+1;
    int h = m_Rect.size.height+1;
    
    level->wrapTerrainTiles(x, y, w, h, type, withType);
}
void Area::setTerrainTile(BaseLevel* level, int x, int y, TerrainTile::TileType type)
{
    if(!level)
        return;
    level->setTerrainTile(x, y, type, m_Type);
}
void Area::setRandomTerrainTile(BaseLevel* level, int m, TerrainTile::TileType type)
{
    if(!level)
        return;
    
    int left = m_Rect.getMinX() + m;
    int right = m_Rect.getMaxX() - m;
    int bottom = m_Rect.getMinY() + m;
    int top = m_Rect.getMaxY() - m;
    int x = cocos2d::random(MIN(left, right), MAX(left, right));
    int y = cocos2d::random(MIN(bottom, top), MAX(bottom, top));
    setTerrainTile(level, x, y, type);
}
int Area::xy2p(const cocos2d::Rect& rect, const cocos2d::Vec2& pos)
{
    int pasWidth = rect.size.width - 2;
    int pasHeight = rect.size.height - 2;
    if (pos.y == rect.getMinY()) {
        
        return (pos.x - rect.getMinX() - 1);
        
    } else if (pos.x == rect.getMaxX()) {
        
        return (pos.y - rect.getMinY() - 1) + pasWidth;
        
    } else if (pos.y == rect.getMaxY()) {
        
        return (rect.getMaxX() - pos.x - 1) + pasWidth + pasHeight;
        
    } else /*if (pos.x == rect.getMinX())*/ {
        
        if (pos.y == rect.getMinY() + 1) {
            return 0;
        } else {
            return (rect.getMaxY() - pos.y - 1) + pasWidth * 2 + pasHeight;
        }
        
    }
}
Vec2 Area::p2xy(const cocos2d::Rect& rect, int p)
{
    int pasWidth = rect.size.width - 2;
    int pasHeight = rect.size.height - 2;
    if (p < pasWidth) {
        
        return Vec2( rect.getMinX() + 1 + p, rect.getMinY() + 1);
        
    } else if (p < pasWidth + pasHeight) {
        
        return Vec2( rect.getMaxX() - 1, rect.getMinY() + 1 + (p - pasWidth) );
        
    } else if (p < pasWidth * 2 + pasHeight) {
        
        return Vec2( rect.getMaxX() - 1 - (p - (pasWidth + pasHeight)), rect.getMaxY() - 1 );
        
    } else {
        
        return Vec2( rect.getMinX() + 1, rect.getMaxY() - 1 - (p - (pasWidth * 2 + pasHeight)) );
        
    }
}