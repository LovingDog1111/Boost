#pragma once
#pragma once
#include "../../Utils/MemoryUtil.h"
#include "HashedString.h"
#include "../../Utils/SHA256.h"
#include "../Render/MatrixStack.h"
#include <json.hpp>
#include <map>
#include <memory>
#include <vector>
#include <variant>
#include <string>
#include <sstream>
#include <iomanip>
#include <atomic>


class ResourceLocation;
class BedrockTexture;
class TextureGroupBase {
public:
    std::map<class ResourceLocation, class BedrockTexture>& getLoadedTextures() {
        return hat::member_at<std::map<class ResourceLocation, class BedrockTexture>>(this, 0x18 + 0x178);
    }
};
namespace mce {
	struct Color {
		union {
			struct {
				float r, g, b, a;
			};
			float arr[4];
		};

		Color(float red = 1.f, float green = 1.f, float blue = 1.f, float alpha = 1.f) {
			this->r = red;
			this->g = green;
			this->b = blue;
			this->a = alpha;
		}
	};


	struct MaterialPtr {
		static MaterialPtr* createMaterial(HashedString const& materialName) {
			static void* materialCreator = (void*)MemoryUtil::getVtableFromSig("48 8B 05 ? ? ? ? 48 8D 55 90 48 8D 0D ? ? ? ? 48 8B 40 08 FF 15 ? ? ? ? 48 8B D8");
			return MemoryUtil::callVirtualFunc<MaterialPtr*, HashedString const&>(1,materialCreator, materialName);
		}//
	};

    class Camera {
    public:
        CLASS_MEMBER(MatrixStack*, viewMatrixStack, 0x0);
        CLASS_MEMBER(glm::mat4x4, viewMatrix, 0xC0);
        CLASS_MEMBER(MatrixStack*, matrixStack, 0x40);
        CLASS_MEMBER(MatrixStack*, projectionMatrixStack, 0x80);
    };

    enum class ImageFormat : int {
        None = 0x0,
        R8Unorm = 0x1,
        RGB8Unorm = 0x2,
        RGBA8Unorm = 0x3,
    };

    class Blob {
    public:
        using value_type = unsigned char;
        using size_type = size_t;
        using pointer = value_type*;
        using iterator = value_type*;
        using const_pointer = value_type const*;
        using const_iterator = value_type const*;

        using delete_function = void (*)(pointer);

        struct Deleter {
        public:
            delete_function mFn;

            [[nodiscard]] _CONSTEXPR23 Deleter() : mFn(Blob::defaultDeleter) {}

            [[nodiscard]] _CONSTEXPR23 Deleter(delete_function fn) : mFn(fn) {}

            void operator()(pointer x) const { mFn(x); }
        };

        using pointer_type = std::unique_ptr<value_type[], Deleter>;

        pointer_type mBlob{}; // this+0x0
        size_type    mSize{}; // this+0x10

        [[nodiscard]] _CONSTEXPR23 Blob() = default;

        [[nodiscard]] _CONSTEXPR23 Blob(std::span<value_type> s, Deleter deleter = {}) : mSize(s.size()) { // NOLINT
            mBlob = pointer_type(new value_type[mSize], deleter);
            std::copy(s.begin(), s.end(), mBlob.get());
        }

        [[nodiscard]] _CONSTEXPR23 pointer data() const { return mBlob.get(); }

        [[nodiscard]] _CONSTEXPR23 size_type size() const { return mSize; }

        [[nodiscard]] _CONSTEXPR23 std::span<value_type> view() const { return { data(), size() }; }

        //LL_CLANG_CEXPR Blob& operator=(Blob&&) noexcept = default;
        [[nodiscard]] _CONSTEXPR23 Blob(Blob&&) noexcept = default;

        [[nodiscard]] _CONSTEXPR23 Blob(Blob const& other) : Blob(other.view(), other.mBlob.get_deleter()) {}

        _CONSTEXPR23 Blob& operator=(Blob const& other) {
            if (this != &other) {
                mSize = other.mSize;
                mBlob = pointer_type(new value_type[mSize], other.mBlob.get_deleter());
                std::copy(other.data(), other.data() + mSize, mBlob.get());
            }
            return *this;
        }

        static Blob fromVector(std::vector<unsigned char> const& vec) {
            Blob blob;
            blob.mSize = vec.size();
            blob.mBlob = pointer_type(new value_type[blob.mSize], Deleter());
            std::copy(vec.begin(), vec.end(), blob.mBlob.get());
            return blob;

        }

    public:
        static inline void defaultDeleter(pointer ptr) { delete[] ptr; }
    };

    enum class ImageUsage : unsigned char {
        Unknown = 0x0,
        sRGB = 0x1,
        Data = 0x2,
    };

    struct Image {
    public:
        ImageFormat imageFormat{}; // this+0x0
        unsigned int        mWidth{};      // this+0x4
        unsigned int        mHeight{};     // this+0x8
        unsigned int        mDepth{};      // this+0xC
        ImageUsage  mUsage{};      // this+0x10
        Blob        mImageBytes;   // this+0x18

        [[nodiscard]] constexpr Image() = default;
        //LL_CLANG_CEXPR Image& operator=(Image&&) noexcept = default;
        [[nodiscard]] constexpr Image(Image&&) noexcept = default;
        //LL_CLANG_CEXPR Image& operator=(Image const&) noexcept = default;
        [[nodiscard]] constexpr Image(Image const&) noexcept = default;
    };


    struct UUID {
        uint64_t mLow;
        uint64_t mHigh;

        static UUID generate()
        {
            UUID uuid = {};
            uuid.mLow = rand();
            uuid.mHigh = rand();
            return uuid;
        }

        [[nodiscard]] std::string toString() const
        {
            // UUID parts
            uint32_t timeLow = (mLow & 0xFFFFFFFF);
            uint16_t timeMid = (mLow >> 32) & 0xFFFF;
            uint16_t timeHiAndVersion = ((mLow >> 48) & 0x0FFF) | (4 << 12); // Set version to 4
            uint16_t clkSeq = (mHigh & 0x3FFF) | 0x8000; // Set variant to 2
            uint16_t nodeHi = (mHigh >> 16) & 0xFFFF;
            uint32_t nodeLow = (mHigh >> 32);

            std::stringstream ss;
            ss << std::hex << std::setfill('0')
                << std::setw(8) << timeLow << '-'
                << std::setw(4) << timeMid << '-'
                << std::setw(4) << timeHiAndVersion << '-'
                << std::setw(4) << clkSeq << '-'
                << std::setw(4) << nodeHi
                << std::setw(8) << nodeLow;

            return ss.str();
        }

        [[nodiscard]] bool operator==(const UUID& other) const
        {
            return mLow == other.mLow && mHigh == other.mHigh;
        }

        [[nodiscard]] bool operator!=(const UUID& other) const
        {
            return !(*this == other);
        }

        [[nodiscard]] bool operator<(const UUID& other) const
        {
            return mLow < other.mLow || (mLow == other.mLow && mHigh < other.mHigh);
        }

        [[nodiscard]] bool operator>(const UUID& other) const
        {
            return mLow > other.mLow || (mLow == other.mLow && mHigh > other.mHigh);
        }

        [[nodiscard]] bool operator<=(const UUID& other) const
        {
            return mLow < other.mLow || (mLow == other.mLow && mHigh <= other.mHigh);
        }

        [[nodiscard]] bool operator>=(const UUID& other) const
        {
            return mLow > other.mLow || (mLow == other.mLow && mHigh >= other.mHigh);
        }

        [[nodiscard]] bool operator!() const
        {
            return mLow == 0 && mHigh == 0;
        }

        [[nodiscard]] explicit operator bool() const
        {
            return mLow != 0 || mHigh != 0;
        }

        static UUID fromString(const std::string& str) {
            UUID uuid;
            std::string hashed = SHA256::hash(str);
            uuid.mLow = std::stoull(hashed.substr(0, 16), nullptr, 16);
            uuid.mHigh = std::stoull(hashed.substr(16, 16), nullptr, 16);
            return uuid;
        }
    };


        class ClientTexture {
        public:
            std::shared_ptr<class data> mResourcePointerBlock;
            //std::shared_ptr<mce::ResourceBlockTemplate<std::variant<std::monostate, mce::Texture, dragon::res::ClientTexture>, mce::PerFrameHandleTracker, mce::ResourceServiceTextureDescription>> mResourcePointerBlock;
        public:
            virtual ~ClientTexture() = default;

            bool operator==(const ClientTexture& other) const {
                return mResourcePointerBlock == other.mResourcePointerBlock;
            }

            bool operator!=(const ClientTexture& other) const {
                return mResourcePointerBlock != other.mResourcePointerBlock;
            }
        };


}


namespace mce {
    enum TextureFormat : uint32_t {
        UNKNOWN_TEXTURE_FORMAT = 0,
        R32G32B32A32_FLOAT = 0x2,
        R16G16B16A16_FLOAT = 0xA,
        R16G16B16A16_UNORM = 0xB,
        R32G32_FLOAT = 0x10,
        R10G10B10A2_UNORM = 0x18,
        R11G11B10_FLOAT = 0x1A,
        R8G8B8A8_UNORM = 0x1C,
        R8G8B8A8_UNORM_SRGB = 0x1D,
        R16G16_FLOAT = 0x22,
        R16G16_UNORM = 0x23,
        R16G16_UINT = 0x24,
        R16G16_SNORM = 0x25,
        D32_FLOAT = 0x28,
        R32_FLOAT = 0x29,
        R32_UINT = 0x2A,
        R24G8_TYPELESS = 0x2C,
        D24_UNORM_S8_UINT = 0x2D,
        R24_UNORM_X8_TYPELESS = 0x2E,
        R8G8_UNORM = 0x31,
        R8G8_SNORM = 0x33,
        R16_FLOAT = 0x36,
        D16_UNORM = 0x37,
        R8_UNORM = 0x3D,
        R8_UINT = 0x3E,
        A8_UNORM = 0x41,
        BC3_UNORM = 0x4D,
        R5G6B5_UNORM = 0x55,
        R5G5B5A1_UNORM = 0x56,
        B8G8R8A8_UNORM = 0x57,
        B8G8R8A8_UNORM_SRGB = 0x5B,
        BC7_UNORM = 0x62,
        R4G4B4A4_UNORM = 0x73,
        S8_UINT = 0x74,
        R8G8B8_UNORM = 0x75,
        COMPRESSED = 0x76
    };
}

namespace cg {
    struct ImageDescription {
        uint32_t           mWidth;
        uint32_t           mHeight;
        mce::TextureFormat mTextureFormat;
        uint32_t           mArraySize;
    };
}

namespace cg {
    class TextureDescription : public cg::ImageDescription {
    public:
        uint32_t mMipCount;
    };
}

namespace cg {
    enum TextureSetLayerType : int32_t {
        Color = 0x0,
        ColorUnlit = 0x1,
        MER = 0x2,
        Metalness = 0x3,
        Emissive = 0x4,
        Roughness = 0x5,
        Normal = 0x6,
        Heightmap = 0x7,
    };
}

class ColorChannel {
    float mValue;
};

namespace cg {
    class TextureSetImageDescription {
    public:
        class LayerInfoVar {
            cg::TextureSetLayerType mLayerType;
            std::variant <cg::ImageDescription, ColorChannel, mce::Color> mData;
        };

        std::vector <cg::TextureSetImageDescription::LayerInfoVar, std::allocator<cg::TextureSetImageDescription::LayerInfoVar>> mLayerInfo;
    };
}

namespace mce {
    enum BindFlagsBit : uint32_t {
        NoBindFlags = 0x0,
        VertexBufferBit = 0x1,
        IndexBufferBit = 0x2,
        ConstantBufferBit = 0x4,
        ShaderResourceBit = 0x8,
        StreamOutputBit = 0x10,
        RenderTargetBit = 0x20,
        DepthStencilBit = 0x40,
        UnorderedAccessBit = 0x80,
    };
}

namespace mce {
    class SampleDescription {
        int32_t mCount;
        int32_t mQuality;
    };
}

namespace mce {
    class TextureDescription : public cg::TextureDescription {
    public:
        mce::SampleDescription mSampleDescription;
        mce::Color mClearColor;
        float mOptimizedClearDepth;
        unsigned char mOptimizedClearStencil;
        mce::BindFlagsBit mBindFlags;
        bool mIsStaging;
    };
}

namespace mce {
    class TextureBase { /* Size=0x78 */
    public:
        mce::TextureDescription textureDescription; // this+0x0
        bool                    created;            // this+0x70
        bool                    mOwnsResource;      // this+0x71
    };
}

namespace mce {
    class TextureNull : public mce::TextureBase {};
}

namespace mce {
    class ResourceBase {};
}

namespace mce {
    class DragonLifetime {};
}

namespace mce
{
    class Texture : public mce::TextureNull, protected mce::ResourceBase {
    public:
        std::shared_ptr<mce::DragonLifetime> mDragonTextureLifetime;
    };
}

namespace mce {
    struct ResourceServiceTextureDescription : public mce::TextureDescription { /* Size=0x90 */
        std::basic_string<char, std::char_traits<char>, std::allocator<char> > mDebugName;
    };
}

namespace mce {
    class PerFrameHandleTracker { /* Size=0x4 */
        std::atomic<unsigned short> mCheckCount;
        std::atomic<bool>           mIsValid;
    };
}



namespace mce {
    class TextureGroup : public TextureGroupBase {};
}

namespace mce {

    namespace MinecraftJson {
enum class ValueType : int32_t {
    Null = 0x0,
    Int = 0x1,
    Uint = 0x2,
    Real = 0x3,
    String = 0x4,
    Boolean = 0x5,
    Array = 0x6,
    Object = 0x7,
};

using ArrayIndex = unsigned int;

class Value {  // equivalent to bds' Json::Value, but we dont wanna confuse it with nlohmann json
   public:
    class CZString {
       public:
        enum class DuplicationPolicy : unsigned int {
            noDuplication = 0,
            duplicate = 1,
            duplicateOnCopy = 2,
            Mask = 3,
        };
        struct StringStorage {
            DuplicationPolicy policy_ : 2;
            unsigned int length_ : 30;  // 1GB max
        };

        char const* cstr_;
        union {
            ArrayIndex index_{};
            StringStorage storage_;
        };

        CZString(ArrayIndex index) : index_(index) {}

        CZString(char const* str, unsigned int length, DuplicationPolicy allocate) : cstr_(str) {
            storage_.policy_ = allocate;
            storage_.length_ = length & 0x3FFFFFFF;
        }
        constexpr CZString& operator=(CZString const& other) {
            if(this == &other) {
                return *this;
            }
            cstr_ = other.cstr_;
            index_ = other.index_;
            return *this;
        }

        [[nodiscard]] constexpr std::strong_ordering operator<=>(CZString const& other) const {
            if(!cstr_)
                return index_ <=> other.index_;
            unsigned int this_len = this->storage_.length_;
            unsigned int other_len = other.storage_.length_;
            unsigned int min_len = std::min<unsigned int>(this_len, other_len);
            int comp = memcmp(this->cstr_, other.cstr_, min_len);

            if(comp == 0)
                return this_len <=> other_len;
            return comp <=> 0;
        }

        [[nodiscard]] constexpr ArrayIndex index() const {
            return index_;
        }
        [[nodiscard]] constexpr char const* c_str() const {
            return cstr_;
        }
        [[nodiscard]] constexpr bool isStaticString() const {
            return storage_.policy_ == DuplicationPolicy::noDuplication;
        }
    };

    /*
    Null    = 0x0,
    Int     = 0x1,
    Uint    = 0x2,
    Real    = 0x3,
    String  = 0x4,
    Boolean = 0x5,
    Array   = 0x6,
    Object  = 0x7,*/

    union ValueHolder {                   // tozic i dont think u understand how unions work
        int64_t mInt;                     // this+0x0
        uint64_t mUInt64;                 // this+0x0
        long double mReal;                // this+0x0
        bool mBool;                       // this+0x0
        char* mString;                    // this+0x0
        std::map<CZString, Value>* mMap;  // this+0x0
    };

    ValueHolder mValue;  // this+0x0000

    ValueType mType : 8;     // this+0x0028
    int32_t mAllocated : 1;  // this+0x002C
   public:
    [[nodiscard]] std::string toString() {
        std::string result;
        switch(mType) {
            case ValueType::Null:
                result = "[nulltype]";
                break;
            case ValueType::Int:
                result = std::to_string(mValue.mInt);
                break;
            case ValueType::Uint:
                result = std::to_string(mValue.mUInt64);
                break;
            case ValueType::Real:
                result = std::to_string(mValue.mReal);
                break;
            case ValueType::String:
                result = std::string("\"") + mValue.mString + std::string("\"");
                break;
            case ValueType::Boolean:
                result = mValue.mBool ? "true" : "false";
                break;
            case ValueType::Array:
                result = "[size: ";
                result += std::to_string(mValue.mMap->size());
                result += "]";
                break;
            case ValueType::Object:
                result = "[object]";
                break;
            default:
                result = "what?";
                break;
        }
        return result;
    }

    std::string toStyledString(std::string* output) {
        /*static uintptr_t func = SigManager::JSON_toStyledString;
        MemUtils::callFastcall<void, void*, std::string*>(func, this, output);
        return *output;*/
    }

    nlohmann::json toNlohmannJson() {
        /*std::string styledString;
        this->toStyledString(&styledString);
        return nlohmann::json::parse(styledString);*/
    }

    static inline MinecraftJson::Value fromNlohmannJson(nlohmann::json const& json) {}
    Value() {
        memset(this, 0, sizeof(*this));
    }
};

static_assert(sizeof(Value) == 0x10, "Value size is invalid");
};


enum class AnimatedTextureType : unsigned int {
    None = 0,
    Face = 1,
    Body32x32 = 2,
    Body128x128 = 3,
};

enum class AnimationExpression : unsigned int {
    Linear = 0,
    Blinking = 1,
};

class AnimatedImageData {
   public:
    AnimatedTextureType mType;
    AnimationExpression mExpression;
    Image mImage;
    alignas(4) float mFrames;
};

class MinEngineVersion {
   public:
    std::aligned_storage_t<24, 8> mUnkce6b03;
    std::aligned_storage_t<4, 4> mUnk7524e3;
    std::aligned_storage_t<2, 2> mUnk2c6d18;
};

namespace persona {
enum class PieceType : unsigned int {
    Unknown = 0,
    Skeleton = 1,
    Body = 2,
    Skin = 3,
    Bottom = 4,
    Feet = 5,
    Dress = 6,
    Top = 7,
    HighPants = 8,
    Hands = 9,
    Outerwear = 10,
    FacialHair = 11,
    Mouth = 12,
    Eyes = 13,
    Hair = 14,
    Hood = 15,
    Back = 16,
    FaceAccessory = 17,
    Head = 18,
    Legs = 19,
    LeftLeg = 20,
    RightLeg = 21,
    Arms = 22,
    LeftArm = 23,
    RightArm = 24,
    Capes = 25,
    ClassicSkin = 26,
    Emote = 27,
    Unsupported = 28,
    Count = 29,
};
}
class TintMapColor {
   public:
    std::aligned_storage_t<64, 4> colors;
};
class SerializedPersonaPieceHandle {
   public:
    std::string mPieceId;
    persona::PieceType mPieceType;
    UUID mPackId;
    bool mIsDefaultPiece;
    std::string mProductId;
};
namespace persona {
class ArmSize {
   public:
    // ArmSize inner types define
    enum class Type : uint64_t {
        Slim = 0,
        Wide = 1,
        Count = 2,
        Unknown = 3,
    };
};
}  // namespace persona

	enum class TrustedSkinFlag : signed char {
    Unset = 0,
    False = 1,
    True = 2,
};
class PlayerSkin {
   public:
    std::string mId;                   // this+0x0
    std::string mPlayFabId;            // this+0x20
    std::string mFullId;               // this+0x40
    std::string mResourcePatch;        // this+0x60
    std::string mDefaultGeometryName;  // this+0x80
    Image mSkinImage;                  // this+0xA0
    Image mCapeImage;                  // this+0xD0

    std::vector<AnimatedImageData> mSkinAnimatedImages;
    MinecraftJson::Value mGeometryData;
    MinEngineVersion mGeometryDataMinEngineVersion;
    MinecraftJson::Value mGeometryDataMutable;
    std::string mAnimationData;
    std::string mCapeId;
    std::vector<SerializedPersonaPieceHandle> mPersonaPieces;
    persona::ArmSize::Type mArmSizeType;
    std::unordered_map<persona::PieceType, TintMapColor> mPieceTintColors;
    Color mSkinColor;
    TrustedSkinFlag mIsTrustedSkin;
    bool mIsPremium;
    bool mIsPersona;
    bool mIsPersonaCapeOnClassicSkin;
    bool mIsPrimaryUser;
    bool mOverridesPlayerAppearance;
};

}


