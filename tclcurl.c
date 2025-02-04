#include <tcl.h>
#include <string.h>
#include <curl/curl.h>
#include "tclcurl.h"


size_t write_callback(char *ptr, size_t size, size_t nmemb, void *clientdata) {
	Tcl_Obj * data = clientdata;
	Tcl_AppendToObj(data,ptr,size*nmemb);
	return size*nmemb;

}


static void Curl_AppendSupportedOptions(Tcl_Obj * obj) {
	Tcl_AppendStringsToObj(obj,
			"customrequest ",
			"httpheader ",
			"keypasswd ",
			"post ",
			"postfields ",
			"sslcert ",
			"sslcerttype ",
			"userpwd ",
			"verbose",
			NULL);
}


static int 
Curl_OptionsCmd(
	void *dummy,	/* Not used. */
    Tcl_Interp *interp,		/* Current interpreter */
    int objc,			/* Number of arguments */
    Tcl_Obj *const objv[]	/* Argument strings */
    )
{
	Tcl_Obj * result = Tcl_NewStringObj("",-1);
	Curl_AppendSupportedOptions(result);
	Tcl_SetObjResult(interp, result);
	return TCL_OK;
	
}


static int
Curl_Cmd(
    void *dummy,	/* Not used. */
    Tcl_Interp *interp,		/* Current interpreter */
    int objc,			/* Number of arguments */
    Tcl_Obj *const objv[]	/* Argument strings */
    )
{
	int return_code = TCL_OK;
	CURLcode res;
	struct curl_slist *headers = NULL;
	if (objc < 2 || objc > 3) {
		Tcl_WrongNumArgs(interp,1,objv,"url ?opts?") ;
		return TCL_ERROR;
	}
	Tcl_Obj * content = Tcl_NewStringObj("",-1);
	Tcl_Obj * resDict;
	CURL *curl = curl_easy_init();
	
	SETSTRINGOPT(curl,interp,CURLOPT_URL,objv[1]);

	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,write_callback);
	if (res != CURLE_OK) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(curl_easy_strerror(res),-1));
		return_code = TCL_ERROR;
		goto cleanup;

	}
	res = curl_easy_setopt(curl, CURLOPT_WRITEDATA,content);
	if (res != CURLE_OK) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(curl_easy_strerror(res),-1));
		return_code = TCL_ERROR;
		goto cleanup;

	}
	// set provided options
	if (objc == 3) {
		Tcl_Obj * key;
		Tcl_Obj * value;
		Tcl_DictSearch search;
		int done;
		if (Tcl_DictObjFirst(interp,objv[2],&search,&key,&value,&done) != TCL_OK) {
			return_code = TCL_ERROR;
			goto cleanup;
		}
		while (done == 0) {
			char * optStr = Tcl_GetString(key);
			if (strcmp(optStr,"postfields") == 0) {
				Tcl_Size bSize;
				char * bytes = Tcl_GetStringFromObj(value,&bSize);
				res = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,bSize);
				if (res != CURLE_OK) {
					Tcl_SetObjResult(interp, Tcl_NewStringObj(curl_easy_strerror(res),-1));
					return_code = TCL_ERROR;
					goto cleanup;

				}
				res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS,bytes);
				if (res != CURLE_OK) {
					Tcl_SetObjResult(interp, Tcl_NewStringObj(curl_easy_strerror(res),-1));
					return_code = TCL_ERROR;
					goto cleanup;

				}

			} else if (strcmp(optStr,"httpheader") == 0) {
				Tcl_Size hobjc;
				Tcl_Obj **  hobjv;
				if (Tcl_ListObjGetElements(interp, value, &hobjc, &hobjv) != TCL_OK) {
					return_code = TCL_ERROR;
					goto cleanup;
				}
				for (int i = 0 ; i < hobjc; i++) {
					headers = curl_slist_append(headers, Tcl_GetString(hobjv[i]));
				}
				if (headers != NULL) {
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
				}
			} else if (strcmp(optStr,"keypasswd") == 0) {
				SETSTRINGOPT(curl,interp,CURLOPT_KEYPASSWD,value);
			} else if (strcmp(optStr,"post") == 0) {
				SETLONGOPT(curl,interp,CURLOPT_POST,value);
			} else if (strcmp(optStr,"sslcert") == 0) {
				SETSTRINGOPT(curl,interp,CURLOPT_SSLCERT,value);
			} else if (strcmp(optStr,"sslcerttype") == 0) {
				SETSTRINGOPT(curl,interp,CURLOPT_SSLCERTTYPE,value);
			} else if (strcmp(optStr,"customrequest") == 0) {
				SETSTRINGOPT(curl,interp,CURLOPT_CUSTOMREQUEST,value);
			} else if (strcmp(optStr,"verbose") == 0) {
				SETLONGOPT(curl,interp,CURLOPT_VERBOSE,value);
			} else if (strcmp(optStr,"userpwd") == 0) {
				SETSTRINGOPT(curl,interp,CURLOPT_USERPWD,value);
			} else {
				Tcl_Obj * error = Tcl_NewStringObj("unsupported option: ",-1);
				Tcl_AppendObjToObj(error,key);
				Tcl_AppendStringsToObj(error, " valid options are: ", NULL);
				Curl_AppendSupportedOptions(error);
				Tcl_SetObjResult(interp, error);
				Tcl_DictObjDone(&search);
				return_code = TCL_ERROR;
				goto cleanup;
			}
			Tcl_DictObjNext(&search,&key,&value,&done);

		}
		
	}


	res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(curl_easy_strerror(res),-1));
		return_code = TCL_ERROR;
		goto cleanup;

	}

	// Success
	resDict = Tcl_NewDictObj();
	Tcl_DictObjPut(interp, resDict, Tcl_NewStringObj("content",-1), content);

	GETLONGINFO(curl,interp, resDict,"response_code",CURLINFO_RESPONSE_CODE);
	GETSTRINGINFO(curl,interp, resDict,"effective_method",CURLINFO_EFFECTIVE_METHOD);


	Tcl_SetObjResult(interp, resDict);
cleanup:
	if (headers != NULL) {
		curl_slist_free_all(headers);
	}
	curl_easy_cleanup(curl);
	return return_code;

}



#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */
DLLEXPORT int
Curl_Init(
    Tcl_Interp* interp)		/* Tcl interpreter */
{

    if (Tcl_InitStubs(interp, "9.0", 0) == NULL) {
	return TCL_ERROR;
    }


    if (Tcl_PkgProvideEx(interp, PACKAGE_NAME, PACKAGE_VERSION, NULL) != TCL_OK) {
	return TCL_ERROR;
    }
    Tcl_CreateObjCommand(interp, "curl::curl", (Tcl_ObjCmdProc *)Curl_Cmd,
	    NULL, NULL);
	Tcl_CreateObjCommand(interp, "curl::options", (Tcl_ObjCmdProc *)Curl_OptionsCmd,
	    NULL, NULL);	
	
   // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    return TCL_OK;
}
#ifdef __cplusplus
}
#endif  /* __cplusplus */
