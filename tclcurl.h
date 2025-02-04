#ifndef TCLCURL_H
#define TCLCURL_H

#define GETLONGINFO(handle, interp, dict, key, info) do {\
	long lVar;\
	if (curl_easy_getinfo((handle), (info), &lVar) == CURLE_OK) {\
	       	Tcl_DictObjPut((interp), (dict), Tcl_NewStringObj((key),-1), Tcl_NewLongObj(lVar));\
	}\
	} while (0)

#define GETSTRINGINFO(handle, interp, dict, key, info) do {\
	char *  ptr;\
	if (curl_easy_getinfo((handle), (info), &ptr) == CURLE_OK) {\
	       	Tcl_DictObjPut((interp), (dict), Tcl_NewStringObj((key),-1), Tcl_NewStringObj(ptr,-1));\
	}\
	} while (0)

#define SETSTRINGOPT(handle,interp,opt,obj) do {\
	CURLcode res = curl_easy_setopt((handle), (opt),Tcl_GetString((obj)));\
	if (res != CURLE_OK) {\
		Tcl_SetObjResult((interp), Tcl_NewStringObj(curl_easy_strerror(res),-1));\
		return_code = TCL_ERROR;\
		goto cleanup;\
	}\
	} while (0)


#define SETLONGOPT(handle,interp,opt,obj) do {\
	long lVar;\
	if (Tcl_GetLongFromObj((interp),(obj), &lVar) != TCL_OK) {\
		return_code = TCL_ERROR;\
		goto cleanup;\
	}\
	CURLcode res = curl_easy_setopt((handle), (opt),lVar);\
	if (res != CURLE_OK) {\
		Tcl_SetObjResult((interp), Tcl_NewStringObj(curl_easy_strerror(res),-1));\
		return_code = TCL_ERROR;\
		goto cleanup;\
	}\
	} while (0)

#endif // TCLCURL_H