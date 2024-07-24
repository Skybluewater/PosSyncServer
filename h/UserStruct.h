//
//  UserStruct.h
//  SocketServerr2.0
//
//  Created by 崔程远 on 2023/5/26.
//

#ifndef UserStruct_h
#define UserStruct_h

#include <string>

struct UserStruct {
    int user_id;
    int pos_x;
    int pox_y;
    int pos_z;
    int rot_x;
    int rot_y;
    int rot_z;
    int level_id;
    bool interact;
    bool isTest;
};

#endif /* UserStruct_h */
