#pragma once
#include <cstdint>
#include <vcruntime_string.h>
class ItemStackNetIdVariant {
  public:
      int id1;
      char sb[4];
      int id2;
      char sb1[4];
      uint8_t type;
      char sb2[7];
  
      ItemStackNetIdVariant() {
          memset(this, 0, sizeof(ItemStackNetIdVariant));
      }
  
      __forceinline int getID() {
          if (type == 0) return id1;
          else if (type <= 2) return id2;
          else return 0;
      }
  
      __forceinline void setID(int id) {
          if (type == 0) id1 = id;
          else if (type <= 2) id2 = id;
      }
  
      ItemStackNetIdVariant(int id) {
          memset(this, 0, sizeof(ItemStackNetIdVariant));
          setID(id);
      }
  };