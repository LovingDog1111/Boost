#pragma once
enum class InventorySourceType : int {
    InvalidInventory = -1,
    ContainerInventory = 0,
    GlobalInventory = 1,
    WorldInteraction = 2,
    CreativeInventory = 3,
    NonImplementedFeatureTODO = 99999,
};


class InventorySource {
public:
    // InventorySource inner types define
    enum class InventorySourceFlags : unsigned int {
        NoFlag = 0x0,
        WorldInteraction_Random = 0x1,
    };

    InventorySourceType  mType = InventorySourceType::InvalidInventory; // this+0x0
    char                 mContainerId = -1;                                     // this+0x4
    InventorySourceFlags mFlags = InventorySourceFlags::NoFlag;          // this+0x8

    inline bool operator==(InventorySource const& rhs) const {
        return ((this->mType == rhs.mType) &&
            (this->mContainerId == rhs.mContainerId) &&
            (this->mFlags == rhs.mFlags));
    }
    inline bool operator!=(InventorySource const& rhs) const {
        return !(*this == rhs);
    }

    inline bool operator<(InventorySource const& rhs) const {
        if (this->mType != rhs.mType) {
            return this->mType < rhs.mType;
        }
        if (this->mContainerId != rhs.mContainerId) {
            return this->mContainerId < rhs.mContainerId;
        }
        return this->mFlags < rhs.mFlags;
    }

    inline bool operator>(InventorySource const& rhs) const {
        return rhs < *this;
    }

};

namespace std
{
    template <>
    struct hash<InventorySource> {
        size_t operator()(InventorySource const& key) const {
            return static_cast<size_t>(key.mContainerId)
                ^ (static_cast<size_t>(static_cast<unsigned int>(key.mType)) << 16);
        }
    };
}