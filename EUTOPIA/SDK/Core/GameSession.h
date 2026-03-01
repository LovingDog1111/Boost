#pragma once
class GameSession {
public:
    void* EventCallback() {
     return hat::member_at<void*>(this,0x50);
    }

};