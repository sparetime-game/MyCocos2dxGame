//
//  HueSprite.cpp
//  MazeJump
//
//  Created by wang haibo on 15/8/12.
//
//

#include "HueSprite.h"
USING_NS_CC;

const GLchar* colorRotationShaderBody();
void xRotateMat(float mat[3][3], float rs, float rc);
void yRotateMat(float mat[3][3], float rs, float rc);
void zRotateMat(float mat[3][3], float rs, float rc);
void matrixMult(float a[3][3], float b[3][3], float c[3][3]);
void hueMatrix(GLfloat mat[3][3], float angle);
void premultiplyAlpha(GLfloat mat[3][3], float alpha);

HueSprite* HueSprite::create(const std::string& filename)
{
    HueSprite *sprite = new (std::nothrow) HueSprite();
    if (sprite && sprite->initWithFile(filename))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

HueSprite* HueSprite::create(const std::string& filename, const cocos2d::Rect& rect)
{
    HueSprite *sprite = new (std::nothrow) HueSprite();
    if (sprite && sprite->initWithFile(filename, rect))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

HueSprite* HueSprite::createWithTexture(cocos2d::Texture2D *texture)
{
    HueSprite *sprite = new (std::nothrow) HueSprite();
    if (sprite && sprite->initWithTexture(texture))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

HueSprite* HueSprite::createWithTexture(cocos2d::Texture2D *texture, const cocos2d::Rect& rect, bool rotated)
{
    HueSprite *sprite = new (std::nothrow) HueSprite();
    if (sprite && sprite->initWithTexture(texture, rect, rotated))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

HueSprite* HueSprite::createWithSpriteFrame(cocos2d::SpriteFrame *spriteFrame)
{
    HueSprite *sprite = new (std::nothrow) HueSprite();
    if (sprite && spriteFrame && sprite->initWithSpriteFrame(spriteFrame))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

HueSprite* HueSprite::createWithSpriteFrameName(const std::string& spriteFrameName)
{
    cocos2d::SpriteFrame *frame = cocos2d::SpriteFrameCache::getInstance()->getSpriteFrameByName(spriteFrameName);
    
#if COCOS2D_DEBUG > 0
    char msg[256] = {0};
    sprintf(msg, "Invalid spriteFrameName: %s", spriteFrameName.c_str());
    CCASSERT(frame != nullptr, msg);
#endif
    
    return createWithSpriteFrame(frame);
}

bool HueSprite::initWithTexture(cocos2d::Texture2D *texture, const cocos2d::Rect &rect, bool rotated)
{
    Texture2D::TexParams tRepeatParams;
    tRepeatParams.magFilter = GL_NEAREST;
    tRepeatParams.minFilter = GL_NEAREST;
    tRepeatParams.wrapS = GL_CLAMP_TO_EDGE;
    tRepeatParams.wrapT = GL_CLAMP_TO_EDGE;
    texture->setTexParameters(tRepeatParams);
    
    bool ret = Sprite::initWithTexture(texture, rect, rotated);
    if(ret)
    {
        setupDefaultSettings();
        initShader();
    }
    return ret;
}

bool HueSprite::initWithTexture(cocos2d::Texture2D *texture)
{
    CCASSERT(texture != nullptr, "Invalid texture for sprite");
    
    cocos2d::Rect rect = cocos2d::Rect::ZERO;
    rect.size = texture->getContentSize();
    
    return initWithTexture(texture, rect);
}

bool HueSprite::initWithTexture(cocos2d::Texture2D *texture, const cocos2d::Rect& rect)
{
    return initWithTexture(texture, rect, false);
}

bool HueSprite::initWithSpriteFrame(cocos2d::SpriteFrame *spriteFrame)
{
    CCASSERT(spriteFrame != nullptr, "");
    
    bool bRet = initWithTexture(spriteFrame->getTexture(), spriteFrame->getRect());
    setSpriteFrame(spriteFrame);
    
    return bRet;
}

void HueSprite::setupDefaultSettings()
{
    _hue = 0.0f;
}

void HueSprite::initShader()
{
    auto glprogram = cocos2d::GLProgramCache::getInstance()->getGLProgram("hue_program");
    if(!glprogram)
    {
        glprogram = cocos2d::GLProgram::createWithByteArrays(cocos2d::ccPositionTextureColor_noMVP_vert, shaderBody());
        cocos2d::GLProgramCache::getInstance()->addGLProgram(glprogram, "hue_program");
    }
    auto glprogramstate = cocos2d::GLProgramState::create(glprogram);
    setGLProgramState(glprogramstate);
    updateColor();
}

const GLchar* HueSprite::shaderBody()
{
    return colorRotationShaderBody();
}

void HueSprite::updateColor()
{
    Sprite::updateColor();
    updateColorMatrix();
    updateAlpha();
}

void HueSprite::hueUniformCallback(cocos2d::GLProgram *p, cocos2d::Uniform *u)
{
    glUniformMatrix3fv(u->location, 1, GL_FALSE, (GLfloat*)&_mat);
}

void HueSprite::updateColorMatrix()
{
    hueMatrix(_mat, _hue);
    premultiplyAlpha(_mat, getAlpha());
    
    getGLProgramState()->setUniformCallback("u_hue", CC_CALLBACK_2(HueSprite::hueUniformCallback, this));
}

void HueSprite::updateAlpha()
{
    getGLProgramState()->setUniformFloat("u_alpha", getAlpha());
}

GLfloat HueSprite::getAlpha()
{
    return _displayedOpacity/255.0f;
}

float HueSprite::getHue()
{
    return _hue;
}

void HueSprite::setHue(float hue)
{
    _hue = hue;
    updateColorMatrix();
}
cocos2d::Rect HueSprite::getBoundingNiloBox() const
{
    V3F_C4B_T2F_Quad quad = getQuad();
    float x = quad.bl.vertices.x;
    float y = quad.bl.vertices.y;
    float width = quad.br.vertices.x - quad.bl.vertices.x;
    float heigth = quad.tl.vertices.y - quad.bl.vertices.y;
    cocos2d::Rect rect(x + 0.25f*width, y, width*0.5f, heigth);
    return RectApplyAffineTransform(rect, getNodeToParentAffineTransform());
}
cocos2d::Rect HueSprite::getBoundingPudgeBox() const
{
    V3F_C4B_T2F_Quad quad = getQuad();
    
       Rect rect(6.0f, 0.0f, _contentSize.width*0.65f, _contentSize.height*0.55f);
    return RectApplyAffineTransform(rect, getNodeToParentAffineTransform());
}
//shader

const GLchar * colorRotationShaderBody()
{
    return
    "                                                               \n\
    #ifdef GL_ES                                                    \n\
    precision mediump float;                                        \n\
    #endif                                                          \n\
    \n\
    varying vec2 v_texCoord;                                        \n\
    uniform mat3 u_hue;                                             \n\
    uniform float u_alpha;                                          \n\
    \n\
    void main()                                                     \n\
    {                                                               \n\
    vec4 pixColor = texture2D(CC_Texture0, v_texCoord);             \n\
    vec3 rgbColor = u_hue * pixColor.rgb;                           \n\
    gl_FragColor = vec4(rgbColor, pixColor.a * u_alpha);            \n\
    }                                                               \n\
    ";
}

void xRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = 1.0;
    mat[0][1] = 0.0;
    mat[0][2] = 0.0;
    
    mat[1][0] = 0.0;
    mat[1][1] = rc;
    mat[1][2] = rs;
    
    mat[2][0] = 0.0;
    mat[2][1] = -rs;
    mat[2][2] = rc;
}

void yRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = 0.0;
    mat[0][2] = -rs;
    
    mat[1][0] = 0.0;
    mat[1][1] = 1.0;
    mat[1][2] = 0.0;
    
    mat[2][0] = rs;
    mat[2][1] = 0.0;
    mat[2][2] = rc;
}


void zRotateMat(float mat[3][3], float rs, float rc)
{
    mat[0][0] = rc;
    mat[0][1] = rs;
    mat[0][2] = 0.0;
    
    mat[1][0] = -rs;
    mat[1][1] = rc;
    mat[1][2] = 0.0;
    
    mat[2][0] = 0.0;
    mat[2][1] = 0.0;
    mat[2][2] = 1.0;
}

void matrixMult(float a[3][3], float b[3][3], float c[3][3])
{
    int x, y;
    float temp[3][3];
    
    for(y=0; y<3; y++) {
        for(x=0; x<3; x++) {
            temp[y][x] = b[y][0] * a[0][x] + b[y][1] * a[1][x] + b[y][2] * a[2][x];
        }
    }
    for(y=0; y<3; y++) {
        for(x=0; x<3; x++) {
            c[y][x] = temp[y][x];
        }
    }
}

void hueMatrix(GLfloat mat[3][3], float angle)
{
#define SQRT_2      sqrt(2.0)
#define SQRT_3      sqrt(3.0)
    
    float mag, rot[3][3];
    float xrs, xrc;
    float yrs, yrc;
    float zrs, zrc;
    
    // Rotate the grey vector into positive Z
    mag = SQRT_2;
    xrs = 1.0/mag;
    xrc = 1.0/mag;
    xRotateMat(mat, xrs, xrc);
    mag = SQRT_3;
    yrs = -1.0/mag;
    yrc = SQRT_2/mag;
    yRotateMat(rot, yrs, yrc);
    matrixMult(rot, mat, mat);
    
    // Rotate the hue
    zrs = sin(angle);
    zrc = cos(angle);
    zRotateMat(rot, zrs, zrc);
    matrixMult(rot, mat, mat);
    
    // Rotate the grey vector back into place
    yRotateMat(rot, -yrs, yrc);
    matrixMult(rot,  mat, mat);
    xRotateMat(rot, -xrs, xrc);
    matrixMult(rot,  mat, mat);
}

void premultiplyAlpha(GLfloat mat[3][3], float alpha)
{
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            mat[i][j] *= alpha;
        }
    }
}