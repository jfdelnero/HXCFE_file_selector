#include <libraries/commodities.h>
#include <proto/commodities.h>

CxObj *HotKey(STRPTR description,struct MsgPort *port,LONG id)
{ CxObj *filter;

  if ((filter=CreateCxObj(CX_FILTER,(LONG)description,NULL))) {
    AttachCxObj(filter,CreateCxObj(CX_SEND,(LONG)port,id));
    AttachCxObj(filter,CreateCxObj(CX_TRANSLATE,NULL,NULL));
    if (CxObjError(filter)) {
      DeleteCxObjAll(filter); filter=NULL;
    }
  }
  return filter;
}
