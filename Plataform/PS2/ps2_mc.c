//PS2_MC.C
#ifdef PS2
#include "ps2_mc.h"
#include "wl_def.h"
#include <stdio.h>

void PS2_Init_Memory_Card_Type()
{
    int ret;
    bool success = true; //Initialize the memory;
    
    ret = mcInit(MC_TYPE_XMC);
    if(!ret)
    {
       printf("Failed to initialize server\n");
       return success = false;
    }

    ret = mcInit(MC_TYPE_PSX);
    if(!ret)
    {
        printf("Failed to initialize PSX Memory Card\n");
        return success = false;
    }

    ret = mcInit(MC_TYPE_PS2);
    if(!ret)
    {
        printf("Failed to initialize PS2 Memory Card\n");
        return success = false;
    }

    ret = mcInit(MC_TYPE_POCKET);
    if(!ret)
    {
        printf("Failed to initialize POCKET Memory Card\n");
        return success = false;
    }   
    
    ret = mcInit(MC_TYPE_NONE);
    if(!ret)
    {
       printf("Unknown type of Memory Card Detected");
       return success = false;
    }
    else
    {
        printf("There´s no memory card inserted");
        return success = false;
    }
}

void PS2_Get_Memory_Card0_Info()
{
    bool success = true;
    int ret;
    int type, free, info;
    ret = mcGetInfo(0, 0, &type, &free, &info);
    if(!ret)
    {
        printf("Failed to get information of the memory card on slot 0");      
        return success = false;
    }
    else
    {
        mcSync(0, NULL, &ret);
        printf("mcGetInfo returned %d\n",ret);
	    printf("Type: %d Free: %d Format: %d\n\n", type, free, info);//Cosmito
        return success = true;
    }
}

void PS2_Get_Memory_Card1_Info()
{
    bool success = true;
    int ret;
    int type, free, info;  
    ret = mcGetInfo(1, 1, &type, &free, &info);
    if(!ret)
    {
        printf("Failed to get information of the memory card on slot 1");      
        return success = false;
    }
    else
    {
        mcSync(0, NULL, &ret);
        printf("mcGetInfo returned %d\n",ret);
	    printf("Type: %d Free: %d Format: %d\n\n", type, free, info);//Cosmito
        return success = true;
    }
}

void PS2_SaveFile_Memory_Card0(const char* filename)
{
    sceMcTblGetDir dir;
    int ret, fd;
    PS2_Init_Memory_Card_Type(); //Get the what type of memory card;
    //ret = mcDetectMemoryCard(0, 0);
    //if(ret == NULL)
    {
        printf("Failed to detect the memory card from Slot 1");
    }
    //else
    {
        //mcGetDir(0, 0, "mc0://Wolf4SDL", NULL, NULL, NULL); //
    }

    fd = open(filename, O_RDWR);
    //TBD: Add write here:
    close(fd);
    mcClose(0);
}

void PS2_SaveFile_Memory_Card1(const char *filename)
{
    sceMcTblGetDir dir;
    int ret, fd;
    PS2_Init_Memory_Card_Type(); //Get the what type of memory card;
    //ret = mcDetectMemoryCard(1, 1);
    //if(ret == NULL)
    {
        printf("Failed to detect the memory card from Slot 2");
    }
    //else
    {
        mcGetDir(0, 0, "mc0://Wolf4SDL", NULL, NULL, NULL); //
    }

    fd = open(filename, O_RDWR);
    //TBD: Add write here:
    close(fd);
    mcClose(0);
}

void PS2_Load_Memory_Card()
{
    PS2_Init_Memory_Card_Type();
    PS2_Get_Memory_Card0_Info();
    PS2_Get_Memory_Card1_Info();
}

void PS2_Unload_Memory_Card()
{
    mcClose(0);
}

#endif