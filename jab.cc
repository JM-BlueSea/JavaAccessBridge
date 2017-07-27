// jab.cc
//author:BlueSea
#include <node.h>
#include <windows.h>

//#include <AccessBridgeCallbacks.h>
#include <AccessBridgePackages.h>

using namespace std;
using namespace v8;

typedef JOBJECT64 AccessibleContext;
typedef JOBJECT64 AccessibleText;
typedef JOBJECT64 AccessibleValue;
typedef JOBJECT64 AccessibleSelection;
typedef JOBJECT64 Java_Object;
typedef JOBJECT64 PropertyChangeEvent;
typedef JOBJECT64 FocusEvent;
typedef JOBJECT64 CaretEvent;
typedef JOBJECT64 MouseEvent;
typedef JOBJECT64 MenuEvent;
typedef JOBJECT64 AccessibleTable;
typedef JOBJECT64 AccessibleHyperlink;
typedef JOBJECT64 AccessibleHypertext;

typedef void (*Windows_runFP) ();
typedef BOOL (*IsJavaWindowFP) (HWND window);
typedef BOOL (*GetAccessibleContextFromHWNDFP) (HWND window, long *vmID, AccessibleContext *ac);
typedef HWND (*getHWNDFromAccessibleContextFP) (long vmID, AccessibleContext ac);
typedef BOOL (*GetAccessibleContextInfoFP) (long vmID, AccessibleContext ac, AccessibleContextInfo *info);
typedef AccessibleContext (*GetAccessibleChildFromContextFP) (long vmID, AccessibleContext ac, jint i);

Windows_runFP Windows_run = NULL;
IsJavaWindowFP IsJavaWindow = NULL;
GetAccessibleContextFromHWNDFP GetAccessibleContextFromHWND = NULL;
getHWNDFromAccessibleContextFP getHWNDFromAccessibleContext = NULL;
GetAccessibleContextInfoFP GetAccessibleContextInfo = NULL;
GetAccessibleChildFromContextFP GetAccessibleChildFromContext = NULL;


HINSTANCE theAccessBridgeInstance = NULL;

#define LOAD_FP(result, type, name) \
	if((result = (type) ::GetProcAddress(theAccessBridgeInstance, name)) == NULL) {return;}

BOOL bInited = FALSE;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

BOOL GetAccessibleContextInfoToObject(long vmID, AccessibleContext ac, Local<Object> obj)
{
	return TRUE;
}

void Exports_initializeAccessBridge(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	if((!bInited) || Windows_run == NULL)
	{
		args.GetReturnValue().Set(Boolean::New(isolate, false));
		return;
	}
	Windows_run();

	const char g_szClassName[] = "myWindowClass";

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = NULL;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        goto exit;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
        "The title of my window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 240, 120,
        NULL, NULL, NULL, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!",
            MB_ICONEXCLAMATION | MB_OK);
        goto exit;
    }

	::PostMessage(hwnd, WM_QUIT, 0, 0);
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }

exit:
	args.GetReturnValue().Set(Boolean::New(isolate, true));
}

void Exports_IsJavaWindow(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);
	if((!bInited) || IsJavaWindow == NULL || (!args[0]->IsNumber()))
	{
		args.GetReturnValue().Set(Boolean::New(isolate, false));
		return;
	}

	args.GetReturnValue().Set(Boolean::New(isolate, IsJavaWindow((HWND)(args[0]->IntegerValue()))));
}
    
void Exports_GetAccessibleContextFromHWND(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	long vmID;
	AccessibleContext ac;
	if((!bInited) || GetAccessibleContextFromHWND == NULL || (!args[0]->IsNumber()) || 
		(!GetAccessibleContextFromHWND((HWND)(args[0]->IntegerValue()), &vmID, &ac)))
	{
		args.GetReturnValue().SetUndefined();
		return;
	}

	Local<Object> obj = v8::Object::New(isolate);
	obj->Set(v8::String::NewFromUtf8(isolate, "vmID"), v8::Uint32::New(isolate, vmID));
	obj->Set(v8::String::NewFromUtf8(isolate, "ac_hi"), v8::Uint32::New(isolate, static_cast<uint32_t>(ac >> 32)));
	obj->Set(v8::String::NewFromUtf8(isolate, "ac_lo"), v8::Uint32::New(isolate, static_cast<uint32_t>(ac & 0xffffffffull)));
	args.GetReturnValue().Set(obj);
}

AccessibleContext getAC(Isolate* isolate, Local<Object> obj)
{
	Local<Uint32> ac_hi = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "ac_hi")));
	Local<Uint32> ac_lo = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "ac_lo")));
	return (AccessibleContext)((static_cast<uint64_t>(ac_hi->Value()) << 32) | (static_cast<uint64_t>(ac_lo->Value())));
}

void Exports_getHWNDFromAccessibleContext(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if((!bInited) || GetAccessibleContextFromHWND == NULL || (!args[0]->IsObject()))
	{
		args.GetReturnValue().Set(Number::New(isolate, 0));
		return;
	}

	Local<Object> obj = Local<Object>::Cast(args[0]);
	Local<Uint32> vmID = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "vmID")));
	long hwnd = (long)getHWNDFromAccessibleContext((long)vmID->Value(), getAC(isolate, obj));
	args.GetReturnValue().Set(Number::New(isolate, hwnd));
}

Local<Object> AccessibleContextInfoToObject(Isolate* isolate, AccessibleContextInfo *info)
{
	Local<Object> obj = v8::Object::New(isolate);

	obj->Set(v8::String::NewFromUtf8(isolate, "name"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->name));
	obj->Set(v8::String::NewFromUtf8(isolate, "description"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->description));

	obj->Set(v8::String::NewFromUtf8(isolate, "role"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->role));
	obj->Set(v8::String::NewFromUtf8(isolate, "role_en_US"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->role_en_US));
	obj->Set(v8::String::NewFromUtf8(isolate, "states"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->states));
	obj->Set(v8::String::NewFromUtf8(isolate, "states_en_US"), v8::String::NewFromTwoByte(isolate, (uint16_t*)info->states_en_US));

	obj->Set(v8::String::NewFromUtf8(isolate, "indexInParent"), v8::Uint32::New(isolate, info->indexInParent));
	obj->Set(v8::String::NewFromUtf8(isolate, "childrenCount"), v8::Uint32::New(isolate, info->childrenCount));

	obj->Set(v8::String::NewFromUtf8(isolate, "x"), v8::Uint32::New(isolate, info->x));
	obj->Set(v8::String::NewFromUtf8(isolate, "y"), v8::Uint32::New(isolate, info->y));
	obj->Set(v8::String::NewFromUtf8(isolate, "width"), v8::Uint32::New(isolate, info->width));
	obj->Set(v8::String::NewFromUtf8(isolate, "height"), v8::Uint32::New(isolate, info->height));

	obj->Set(v8::String::NewFromUtf8(isolate, "accessibleComponent"), v8::Boolean::New(isolate, info->accessibleComponent));
	obj->Set(v8::String::NewFromUtf8(isolate, "accessibleAction"), v8::Boolean::New(isolate, info->accessibleAction));
	obj->Set(v8::String::NewFromUtf8(isolate, "accessibleSelection"), v8::Boolean::New(isolate, info->accessibleSelection));
	obj->Set(v8::String::NewFromUtf8(isolate, "accessibleText"), v8::Boolean::New(isolate, info->accessibleText));

	obj->Set(v8::String::NewFromUtf8(isolate, "accessibleInterfaces"), v8::Boolean::New(isolate, info->accessibleInterfaces));

	return obj;
}

void Exports_GetAccessibleContextInfo(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if((!bInited) || GetAccessibleContextInfo == NULL || (!args[0]->IsObject()))
	{
		args.GetReturnValue().SetUndefined();
		return;
	}

	Local<Object> obj = Local<Object>::Cast(args[0]);
	Local<Uint32> vmID = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "vmID")));
	AccessibleContextInfo aci;
	if(!GetAccessibleContextInfo((long)vmID->Value(), getAC(isolate, obj), &aci))
	{
		args.GetReturnValue().SetUndefined();
		return;
	}

	args.GetReturnValue().Set(AccessibleContextInfoToObject(isolate, &aci));
}

void Exports_GetAcessContextByArray(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if((!bInited) || GetAccessibleChildFromContext == NULL || (!args[0]->IsObject()) || (!args[1]->IsArray()))
	{
		args.GetReturnValue().Set(Uint32::New(isolate, -1));
		return;
	}

	Local<Object> obj = Local<Object>::Cast(args[0]);
	Local<Uint32> vmID = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "vmID")));
	AccessibleContext childAc = getAC(isolate, obj);
	Local<Array> levelArray = Local<Array>::Cast(args[1]);

	int i;
	for (i = 0; i < (int)levelArray->Length() && childAc != 0; i++)
	{
		childAc = GetAccessibleChildFromContext((long)vmID->Value(), childAc, levelArray->Get(i)->Uint32Value());
	}

	AccessibleContextInfo aci;
	if(childAc == 0 || (!GetAccessibleContextInfo((long)vmID->Value(), childAc, &aci)))
	{
		args.GetReturnValue().Set(Uint32::New(isolate, 0));
		return;
	}

	args.GetReturnValue().Set(AccessibleContextInfoToObject(isolate, &aci));
}

AccessibleContext FindAccessContext(long vmID, AccessibleContext parentAC, const wchar_t* name)
{
	AccessibleContextInfo aci ;
	GetAccessibleContextInfo(vmID, parentAC, &aci);
	int i;
	for(i = 0; i < aci.childrenCount; i++)
	{
		AccessibleContext childAC = GetAccessibleChildFromContext(vmID, parentAC, i);
		AccessibleContextInfo acif;
		GetAccessibleContextInfo(vmID, childAC, &acif);
		if(wcscmp(name, acif.name) == 0)
		{
			return childAC;
		}
		else if (acif.childrenCount > 0)
		{
			//如果有子节点 递归
			AccessibleContext acTemp =  FindAccessContext(vmID, childAC, name);
			if (acTemp != 0)
			{
				//如果找到则返回，否则继续找其他兄弟路径
				return acTemp;
			}
		}
	}
	return 0;
}

std::wstring Utf8ToUtf16(const char* u8string)
{
    int wcharcount = strlen(u8string);
    wchar_t *tempWstr = new wchar_t[wcharcount];
    MultiByteToWideChar(CP_UTF8, 0, u8string, -1, tempWstr, wcharcount);
    wstring w(tempWstr);
    delete [] tempWstr;
    return w;
}

void Exports_FindAccessContext(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = Isolate::GetCurrent();
	HandleScope scope(isolate);

	if((!bInited) || GetAccessibleContextInfo == NULL || GetAccessibleChildFromContext == NULL || 
		(!args[0]->IsObject()) || (!args[1]->IsString()))
	{
		args.GetReturnValue().Set(Uint32::New(isolate, -1));
		return;
	}

	Local<Object> obj = Local<Object>::Cast(args[0]);
	Local<Uint32> vmID = Local<Uint32>::Cast(obj->Get(String::NewFromUtf8(isolate, "vmID")));
	String::Utf8Value utfValue(args[1]->ToString());

	AccessibleContextInfo aci;
	AccessibleContext ac = FindAccessContext((long)vmID->Value(), getAC(isolate, obj), Utf8ToUtf16(*utfValue).c_str());
	if(ac == 0 || (!GetAccessibleContextInfo((long)vmID->Value(), ac, &aci)))
	{
		args.GetReturnValue().Set(Uint32::New(isolate, 0));
		return;
	}

	args.GetReturnValue().Set(AccessibleContextInfoToObject(isolate, &aci));
}

void Init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "initializeAccessBridge", Exports_initializeAccessBridge);
  NODE_SET_METHOD(exports, "IsJavaWindow", Exports_IsJavaWindow);
  NODE_SET_METHOD(exports, "GetAccessibleContextFromHWND", Exports_GetAccessibleContextFromHWND);
  NODE_SET_METHOD(exports, "getHWNDFromAccessibleContext", Exports_getHWNDFromAccessibleContext);
  NODE_SET_METHOD(exports, "GetAccessibleContextInfo", Exports_GetAccessibleContextInfo);
  NODE_SET_METHOD(exports, "GetAcessContextByArray", Exports_GetAcessContextByArray);
  NODE_SET_METHOD(exports, "FindAccessContext", Exports_FindAccessContext);
	 
	theAccessBridgeInstance = ::LoadLibrary("WindowsAccessBridge-64.dll");
	if(theAccessBridgeInstance)
	{
		LOAD_FP(Windows_run, Windows_runFP, "Windows_run");
		LOAD_FP(IsJavaWindow, IsJavaWindowFP, "isJavaWindow");
		LOAD_FP(GetAccessibleContextFromHWND, GetAccessibleContextFromHWNDFP, "getAccessibleContextFromHWND");
		LOAD_FP(getHWNDFromAccessibleContext, getHWNDFromAccessibleContextFP, "getHWNDFromAccessibleContext");
		LOAD_FP(GetAccessibleContextInfo, GetAccessibleContextInfoFP, "getAccessibleContextInfo");
		LOAD_FP(GetAccessibleChildFromContext, GetAccessibleChildFromContextFP, "getAccessibleChildFromContext");
	}

	bInited = TRUE;
}

NODE_MODULE(JavaAccessBridge, Init)
