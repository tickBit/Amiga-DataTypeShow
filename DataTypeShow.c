/*
    Shows pictures with datatypes

    Can be compiled with AmiKit's DevPack's GCC
    gcc -lamiga DataTypeShow.c -o DataTypeShow

    I haven't been able to compile this with VBCC...

    It's been a while since I've done Amiga system programming in C,
    the code is quite messy.. Based on an example program in the SDK.
    
 */

#include <stdio.h>
#include <stdlib.h>

#include <exec/memory.h>
#include <dos/dos.h>

#include <datatypes/pictureclass.h>

#include <utility/tagitem.h>

#include <intuition/intuition.h>
#include <intuition/icclass.h>

#include <clib/amigaguide_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

/*
   I guess my system's GCC's includes are older than the ones, that are needed for this.
   This is why I've pasted these from later include files...
*/

#define PDTA_WhichPicture	(DTA_Dummy + 219)
#define PDTA_GetNumPictures	(DTA_Dummy + 220)

/* New for V44. Address of a DTST_MEMORY source type
 * object (APTR).
 */
#define DTA_SourceAddress	(DTA_Dummy+39)

/* New for V44. Size of a DTST_MEMORY source type
 * object (ULONG).
 */
#define DTA_SourceSize		(DTA_Dummy+40)

#define	DTST_MEMORY		5

struct Library * IntuitionBase;
struct Library * DataTypesBase;
struct Library * UtilityBase;


void LoadPic(STRPTR Name)
{
	struct Window *Window = NULL;
    int width, height, newWidth, newHeight;
    struct BitMapHeader  *bmhd;

    struct IntuiMessage *Message; 

	Object *Item;
	APTR memory = NULL;
	LONG size = 0;
	BPTR fileLock;


	fileLock = Lock(Name,SHARED_LOCK);
	
    if(fileLock != NULL)
		{
			struct FileInfoBlock __aligned fib;
			BOOL good = FALSE;

			if(Examine(fileLock,&fib) && fib.fib_DirEntryType < 0 && fib.fib_Size > 0)
			{
				size = fib.fib_Size;
				memory = AllocVec(size,MEMF_ANY|MEMF_PUBLIC);
				if(memory != NULL)
				{
					BPTR fileHandle;

					fileHandle = Open(Name,MODE_OLDFILE);
					if(fileHandle != NULL)
					{
						if(Read(fileHandle,memory,size) == size)
							good = TRUE;

						Close(fileHandle);
					}
				}
			}

			UnLock(fileLock);

			if(!good)
			{
				FreeVec(memory);
				memory = NULL;

				size = 0;

                printf("File not recognized.\n");
				cleanup(memory, Window);
			}
		} else {
            printf("File not found!\n");
            cleanup(memory, Window);
        }

        
        Window = OpenWindowTags(NULL,
		WA_Title,			Name,
		WA_InnerWidth,		400,
		WA_InnerHeight,		400,
		WA_SizeBRight,		TRUE,
		WA_SizeBBottom,		TRUE,
		WA_CloseGadget,		TRUE,
		WA_DepthGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_SizeGadget,		TRUE,
		WA_RMBTrap,			TRUE,
		WA_Activate,		TRUE,
		WA_SimpleRefresh,	TRUE,
		WA_IDCMP,			IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_NEWSIZE |
							IDCMP_GADGETUP | IDCMP_GADGETDOWN | IDCMP_MOUSEBUTTONS |
							IDCMP_MOUSEMOVE | IDCMP_VANILLAKEY | IDCMP_IDCMPUPDATE |
							IDCMP_RAWKEY,
	TAG_DONE);

    if (Window == NULL) cleanup(memory);

    WindowLimits(Window,
			Window->BorderLeft + 100 + Window->BorderRight,Window->BorderTop + 100 + Window->BorderBottom,
			Window->WScreen->Width,Window->WScreen->Height);

	if(memory != NULL)
		{
               
			
			Item = NewDTObject(Name,
				PDTA_WhichPicture,	0,
				PDTA_GetNumPictures, 1,
                DTA_SourceType,      DTST_MEMORY,
				DTA_SourceAddress,	memory,
				DTA_SourceSize,		size,
				GA_Immediate,		TRUE,
				GA_RelVerify,		TRUE,
				DTA_TextAttr,		Window->WScreen->Font,
				GA_Left,			Window->BorderLeft,
				GA_Top,				Window->BorderTop,
				GA_RelWidth,		-(Window->BorderLeft + Window->BorderRight),
				GA_RelHeight,		-(Window->BorderTop + Window->BorderBottom),
				ICA_TARGET,			ICTARGET_IDCMP,
			TAG_DONE);

            printf("Memory object created\n");
		}
		else
		{
			Item = NewDTObject(Name,
				GA_Immediate,	TRUE,
				GA_RelVerify,	TRUE,
				DTA_TextAttr,	Window->WScreen->Font,
				GA_Left,		Window->BorderLeft,
				GA_Top,			Window->BorderTop,
				GA_RelWidth,	-(Window->BorderLeft + Window->BorderRight),
				GA_RelHeight,	-(Window->BorderTop + Window->BorderBottom),
                
				ICA_TARGET,		ICTARGET_IDCMP,
			TAG_DONE);

            printf("File object created\n");
		}


		if(Item != NULL)
		{
			/* struct IntuiMessage *Message; */
			ULONG MsgClass;
			UWORD MsgCode;
			struct TagItem *MsgTags,*List,*This;
			BOOL Done;

        	GetDTAttrs(Item, PDTA_BitMapHeader, &bmhd, TAG_DONE);
        	width = bmhd->bmh_Width;
        	height = bmhd->bmh_Height;
        	newWidth = width + Window->BorderLeft + Window->BorderRight;
        	newHeight = height + Window->BorderTop + Window->BorderBottom;

            printf("Picture dimensions: %d x %d\n", width, height);

            Done = FALSE;

            /* add the item to the Window */
            AddDTObject(Window,NULL,Item,-1);
			RefreshDTObjects(Item,Window,NULL,NULL);

            /* set new window dimensions */
             
            ChangeWindowBox(Window, Window->LeftEdge, Window->TopEdge, newWidth, newHeight);
            
			printf("Event loop\n");

			do
			{
				WaitPort(Window->UserPort);

				while((Message = (struct IntuiMessage *)GetMsg(Window->UserPort)) != NULL)
				{
					MsgClass = Message->Class;
					MsgCode = Message->Code;
					MsgTags = Message->IAddress;

					switch(MsgClass)
					{

						case IDCMP_CLOSEWINDOW:

							Done = TRUE;
							break;

						case IDCMP_IDCMPUPDATE:

							List = MsgTags;

							while((This = NextTagItem(&List)) != NULL)
							{
								switch(This->ti_Tag)
								{
									case DTA_Busy:

										if(This->ti_Data)
										{
											SetWindowPointer(Window,
												WA_BusyPointer,	TRUE,
											TAG_DONE);
										}
										else
										{
											SetWindowPointerA(Window,NULL);
										}

										break;

									case DTA_Sync:

										RefreshDTObjects(Item,Window,NULL,NULL);
										break;
								}
							}

							break;

						case IDCMP_REFRESHWINDOW:

                            printf("Window dimensions: %d x %d\n", Window->Width, Window->Height);
							ReplyMsg((struct Message *)Message);

							BeginRefresh(Window);
							EndRefresh(Window,TRUE);

							Message = NULL;

							break;
					}

					if(Message != NULL)
						ReplyMsg((struct Message *)Message);
				}
			} while(!Done);

			RemoveDTObject(Window,Item);

			DisposeDTObject(Item);

		}

		cleanup(memory, Window);
	}


int main(int argc,char **argv)
{

	/* The program should probably ask for later version of these libraries... */

   	if(IntuitionBase == NULL)
    {
        IntuitionBase = OpenLibrary("intuition.library",0);
        if(IntuitionBase == NULL)
            return -1;
    }

    if (DataTypesBase == NULL)
    {
        DataTypesBase = OpenLibrary("datatypes.library",0);
        if(DataTypesBase == NULL) {
            
            if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);

            return -1;
        }
    }

    if (UtilityBase == NULL)
    {
        UtilityBase = OpenLibrary("utility.library",0);
        if(UtilityBase == NULL) {
            
            if (DataTypesBase != NULL) CloseLibrary(DataTypesBase);
            if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);

            return -1;
        }
    }

	if(argc > 1) LoadPic(argv[1]); else printf("No filename given as parameter.\n");


	return 0;
}

int cleanup(memory, Window) {

    if (DataTypesBase != NULL) CloseLibrary(DataTypesBase);
    if (IntuitionBase != NULL) CloseLibrary(IntuitionBase);
    if (UtilityBase != NULL) CloseLibrary(UtilityBase);

    if (memory != NULL) FreeVec((APTR)memory);

	if (Window != NULL) CloseWindow((struct Window*)Window);
	
	exit(0);

}
