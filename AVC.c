#include"AVC.h"

//end -=3是因为之之前进行了最后两位00的补正
//返回开始位数的前一位的末尾
const uint8_t *ffmage_avc_find_startcode_internal(const uint8_t *ptr,const uint8_t *end){
    //加上4是避免255的这种
    const uint8_t *caddr  = ptr+4 - ((intptr_t)ptr &3);//得到最后两位为00的地址
    //在第一个00之间进行搜索，这是防止遗漏省去3位地址的开始码
    for(end -=3;ptr<caddr && ptr<end;ptr++){
        if(ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 1) //0 0 1
            return ptr;
    }
    //上面没找到001的样式
    //下面开始四位四位的查看
    for(end -=3;ptr<end;ptr+=4){
        uint32_t x = *(const uint32_t*)ptr;//从位置取出来4个字节的数据
        if((x-0x01010101)& (~x)&0x80808080){    //地址中至少要有一个1存在

            if(ptr[1] == 0){
                if(ptr[0] == 0 && ptr[2] == 1) // 0 0 1
                    return ptr;
                if(ptr[2] == 0 && ptr[3] == 1) //x 0 0 1
                    return ptr+1;
            }

            if (ptr[3] == 0) {
                if (ptr[2] == 0 && ptr[4] == 1) // x x 0 0 1
                    return ptr+2;
                if (ptr[4] == 0 && ptr[5] == 1) // x x x 0 0 1
                    return ptr+3;
            }
        }

    }

    //保证255的情况也没问题，回来画个图解释。
    for (end += 3; ptr < end; ptr++) {  //
        if (ptr[0] == 0 && ptr[1] == 0 && ptr[2] == 1) //0 0 1
            return ptr;
    }

    return end + 3; // no start code in [p, end], return end.
}

const uint8_t* ffmage_avc_find_startcode(const uint8_t *ptr,const uint8_t *end){

    const uint8_t *out = ffmage_avc_find_startcode_internal(ptr,end);
    if(ptr < out && out < end && !out[-1]) 
        out--; // find 0001 in x001
    return out;
}