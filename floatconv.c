#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>

uint64_t round_to_even(uint64_t m_data,uint64_t len1,uint64_t len2)
{
    uint64_t part1=m_data>>(len1-len2+1);
    uint64_t part2=m_data & ((uint64_t)1<<(len1-len2+1)-1);

    uint64_t res;

    if(part2<=((uint64_t)1<<(len1-len2-1)))
    {
        res=part1<<1;
    }
    else if(part2>=((uint64_t)1<<(len1-len2-1)))
    {
        res=(part1+1)<<1;
    }
    else{
        res=(part1<<1)+1;
    }

    return res;

}

uint64_t round_to_odd(uint64_t m_data,uint64_t len1,uint64_t len2)
{
    uint64_t part1=m_data>>(len1-len2);
    uint64_t part2=m_data & ((uint64_t)1<<(len1-len2)-1);

    uint64_t res;

    if(part2==0)
    {
        res=part1;
    }
    else{
        res=part1 & 0x1;
    }

    return res;

}
    



float f64_to_f32_even(double f64,uint64_t MODE)
{
    uint64_t uf64=*(uint64_t *)&f64;
    uint64_t sign=uf64>>63;
    uint64_t e_f64=(uf64>>52) & 0x7ff;
    uint64_t m_f64=uf64 & 0xfffffffffffff;

    uint32_t e_f32;
    uint32_t m_f32;

    uint32_t uf32=sign<<31;

    if(e_f64 == 0x7ff)
    {
        if(m_f64 == 0)//inf转inf
        {
            e_f32 = 0xff;
            m_f32 = 0;
        }
        else{//NAN转QNAN
            e_f32 = 0xff;
            m_f32 = round_to_even(m_f64,52,23) | (1<<22);            
        }
    }
    else if(e_f64>1023+126)//溢出到inf
    {
        e_f32=0xff;
        m_f32=0;
    }
    else if(e_f64>1023-127)//e不等于0
    {
        e_f32=e_f64-(1023-127);
        m_f32=round_to_even(m_f64,52,23);
    }
    else if(e_f64<1023-128-23)//过小取0
    {
        e_f32=0;
        m_f32=0;
    }
    else{
        e_f32=0;
        m_f32=round_to_even(m_f64,53+(1023-127-e_f64),23);
    }

    if(m_f32==(1<<23))//当m位溢出
    {
        m_f32==0;
        e_f32+=1;
    }

    uf32= uf32 | (e_f32<<23) | m_f32;

    return *(float *)&uf32;

}