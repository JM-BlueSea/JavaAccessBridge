"use strict"
const script={
	name:'Test',
	author:'BlueSea'
}
const JavaAccessBridge=require('./build/Release/JavaAccessBridge')
const InitializeAccessBridgeResult=JavaAccessBridge.initializeAccessBridge()
console.log('初始化结果: '+InitializeAccessBridgeResult)
const Windowhandle=parseInt('000103E8',16)
console.log("窗口句柄: "+Windowhandle)
const IsJavaWindowTrue=JavaAccessBridge.IsJavaWindow(Windowhandle)
console.log('是否Java界面: '+IsJavaWindowTrue)
const JavaUI=JavaAccessBridge.GetAccessibleContextFromHWND(Windowhandle)
console.log(JavaUI)
const Windowhandle2=JavaAccessBridge.getHWNDFromAccessibleContext(JavaUI).toString()
console.log("反推窗口句柄: "+Windowhandle2)
const JavaUIContextInfo=JavaAccessBridge.GetAccessibleContextInfo(JavaUI)
console.log(JavaUIContextInfo)
const PanelPositionArray=[0,1,0,1]
const target=JavaAccessBridge.GetAcessContextByArray(JavaUI,PanelPositionArray)
console.log(target)
// const button=JavaAccessBridge.FindAccessContext(JavaUI,"点击")
// console.log(button)
// const label=JavaAccessBridge.FindAccessContext(JavaUI,"提示")
// console.log(label)