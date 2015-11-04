//
//  UtilityHelper.cpp
//  
//
//  Created by wang haibo on 15/7/2.
//
//

#include "UtilityHelper.h"
USING_NS_CC;

std::string UtilityHelper::getLocalString(const std::string& key)
{
    ValueMap dict;
    LanguageType lt= CCApplication::getInstance()->getCurrentLanguage();
    switch (lt) {
        case LanguageType::CHINESE:
            dict = FileUtils::getInstance()->getValueMapFromFile("chinese.plist");
            CCASSERT(!dict.empty(), "cannot create dictionary");
            break;
            
        default:
            dict = FileUtils::getInstance()->getValueMapFromFile("english.plist");
            CCASSERT(!dict.empty(), "cannot create dictionary");
            break;
    }
    
    Value ret = dict[key];
    
    return ret.asString();
}
void UtilityHelper::getHexDigest(const unsigned char* md5, int len, std::string& hexStr)
{
    char* hex = new char[len*2+1];
    hex[len*2] =  '\0';
    
    int i;
    static const char HEX_CHARS[]="0123456789abcdef";
    for (i=0;i<len;i++)
    {
        hex[i+i] = HEX_CHARS[md5[i] >> 4];
        hex[i+i+1] = HEX_CHARS[md5[i] & 0x0f];
    }
    hexStr = std::string(hex, 2*len);
    CC_SAFE_DELETE_ARRAY(hex);
}

void UtilityHelper::screenshot(const std::string& fileName,const std::function<void(const std::string&)>& callback)
{
    Image::Format format;
    //进行后缀判断
    if(std::string::npos != fileName.find_last_of(".")){
        auto extension = fileName.substr(fileName.find_last_of("."),fileName.length());
        if (!extension.compare(".png")) {
            format = Image::Format::PNG;
        } else if(!extension.compare(".jpg")) {
            format = Image::Format::JPG;
        } else{
            CCLOGWARN("cocos2d: the image can only be saved as JPG or PNG format");
            return;
        }
    } else {
        CCLOGWARN("cocos2d: the image can only be saved as JPG or PNG format");
        return ;
    }
    //获取屏幕尺寸，初始化一个空的渲染纹理对象
    cocos2d::Size winSize = Director::getInstance()->getWinSize();
    auto renderTexture = RenderTexture::create(winSize.width, winSize.height, Texture2D::PixelFormat::RGBA8888);
    //清空并开始获取
    renderTexture->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
    //遍历场景节点对象，填充纹理到RenderTexture中
    Director::getInstance()->getRunningScene()->visit();
    //结束获取
    renderTexture->end();
    //保存文件
    renderTexture->saveToFile(fileName , format);
    //使用schedule在下一帧中调用callback函数
    auto fullPath = FileUtils::getInstance()->getWritablePath() + fileName;
    auto scheduleCallback = [&,fullPath,callback](float dt){
        callback(fullPath);
    };
    auto _schedule = Director::getInstance()->getRunningScene()->getScheduler();
    _schedule->schedule(scheduleCallback, Director::getInstance(), 0.0f,0,0.0f, false, "screenshot");
}

bool UtilityHelper::checkCircleIntersectWithSegment(const Vec2& center, float radius, const Vec2& start, const Vec2& end)
{
    bool ret = false;
    Vec2 d = (end - start).getNormalized();
    Vec2 e = center - start;
    
    float a = e.dot(d);
    
    float a2 = a*a;
    float e2 = center.distanceSquared(start);
    float r2 = radius*radius;
    
    if ((r2 + a2) < e2)
        ret = false;
    else
    {
        Vec2 v1 = (start - center).getNormalized();
        Vec2 v2 = (end - center).getNormalized();
        float f = v1.dot(v2);
        if(f > 0)
            ret = false;
        else
            ret = true;
    }
    ////这个判断算法只是粗略检测，当两点之间距离是小于半径2倍时会穿透。
    return ret;
}
void UtilityHelper::getCameraToViewportRay(Camera* camera,const Vec2& screenPoint, Ray* outRay)
{
    if(!camera)
        outRay = nullptr;
    else
    {
        auto size = Director::getInstance()->getWinSize();
        float screenX = screenPoint.x / size.width;
        float screenY = screenPoint.y / size.height;
        
        float nx = (2.0f * screenX) -1.0f;
        float ny = (2.0f * screenY) -1.0f;
        
        Vec4 nearPoint(nx, ny, -1.0f, 1.0f);
        Vec4 midPoint(nx, ny, 0.0f, 1.0f);
        
        
        Vec4 origin, target;
        camera->getViewProjectionMatrix().getInversed().transformVector(nearPoint, &origin);
        camera->getViewProjectionMatrix().getInversed().transformVector(midPoint, &target);
        if(origin.w > 0.0f)
        {
            origin.x /= origin.w;
            origin.y /= origin.w;
            origin.z /= origin.w;
            origin.w = 1.0f;
        }
        if(target.w > 0.0f)
        {
            target.x /= target.w;
            target.y /= target.w;
            target.z /= target.w;
            target.w = 1.0f;
        }
        Vec3 dir = Vec3(target.x - origin.x, target.y - origin.y, target.z - origin.z);
        dir.normalize();
        outRay->set(Vec3(origin.x, origin.y, origin.z), dir);
    }

}
Node* UtilityHelper::seekNodeByTag(Node* root, int tag){
    if (!root)
    {
        return nullptr;
    }
    if (root->getTag() == tag)
    {
        return root;
    }
    const auto& arrayRootChildren = root->getChildren();
    ssize_t length = arrayRootChildren.size();
    for (ssize_t i=0;i<length;i++)
    {
        Node* child = dynamic_cast<Node*>(arrayRootChildren.at(i));
        if (child)
        {
            Node* res = seekNodeByTag(child,tag);
            if (res != nullptr)
            {
                return res;
            }
        }
    }
    return nullptr;
}


Node* UtilityHelper::seekNodeByName(Node* root, const std::string& name)
{
    if (!root)
    {
        return nullptr;
    }
    if (root->getName() == name)
    {
        return root;
    }
    const auto& arrayRootChildren = root->getChildren();
    for (auto& subWidget : arrayRootChildren)
    {
        Node* child = dynamic_cast<Node*>(subWidget);
        if (child)
        {
            Node* res = seekNodeByName(child,name);
            if (res != nullptr)
            {
                return res;
            }
        }
    }
    return nullptr;
}
Color3B UtilityHelper::randomColor(int minSum, int minDelta)
{
    int r, g, b, min, max;
    while (true) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        r = arc4random_uniform(255);
        g = arc4random_uniform(255);
        b = arc4random_uniform(255);
#else
        r = arc4random() % 256;
        g = arc4random() % 256;
        b = arc4random() % 256;
#endif
        min = MIN(MIN(r, g), b);
        max = MAX(MAX(r, g), b);
        if (max-min < minDelta) continue;
        if (r+g+b < minSum) continue;
        break;
    }
    return Color3B(r, g, b);
}
Color3B UtilityHelper::randomRedColor()
{
    int r = cocos2d::random(252, 255);
    int g = cocos2d::random(132, 200);
    int b = g;
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomOrangeColor()
{
    int r = cocos2d::random(252, 255);
    int g = cocos2d::random(200, 238);
    int b = g-cocos2d::random(40, 50);
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomYellowColor()
{
    int r = cocos2d::random(252, 255);
    int g = cocos2d::random(252, 255);
    int b = cocos2d::random(103, 203);
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomGreenColor()
{
    int r = cocos2d::random(200, 234);
    int g = cocos2d::random(238, 252);
    int b = r-cocos2d::random(40, 50);
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomCyanColor()
{
    int g = cocos2d::random(238, 250);
    int b = cocos2d::random(252, 255);
    int r = g - cocos2d::random(40, 50);
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomBlueColor()
{
    int g = cocos2d::random(186, 230);
    int b = cocos2d::random(252, 255);
    int r = g - cocos2d::random(40, 50);
    return Color3B(r,g,b);
}
Color3B UtilityHelper::randomPurpleColor()
{
    int r = cocos2d::random(209, 240);
    int g = r -cocos2d::random(40, 50);
    int b = cocos2d::random(252, 255);
    return Color3B(r,g,b);
}