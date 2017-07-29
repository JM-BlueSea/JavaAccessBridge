"use strict"

const JavaAccessBridge=require('./build/Release/JavaAccessBridge')
const InitializeAccessBridgeResult=JavaAccessBridge.initializeAccessBridge()
console.log(`初始化结果:${InitializeAccessBridgeResult?'成功':'失败'}`)
const Windowhandle=parseInt('00030220',16)
console.log(`窗口句柄:${Windowhandle}`)
const IsJavaWindowTrue=JavaAccessBridge.IsJavaWindow(Windowhandle)
console.log(`是否Java界面:${IsJavaWindowTrue?'成功':'失败'}`)
const JavaUI=JavaAccessBridge.GetAccessibleContextFromHWND(Windowhandle)
console.log(JavaUI)
const WindowhandleBack=JavaAccessBridge.getHWNDFromAccessibleContext(JavaUI).toString()
console.log(`反推窗口句柄:${WindowhandleBack}`)

//往text输入文本
// const PanelPositionArray=[0,1,0,0]
// const TextInput=JavaAccessBridge.GetAcessContextByArray(JavaUI,PanelPositionArray)
// console.log(`索引数组定位的对象:\n${JSON.stringify(TextInput)}`)
// const WhetherSetTextSuccess=JavaAccessBridge.setTextContents(TextInput.vmac,'设置文本')
// console.log(`是否设置文本成功:${WhetherSetTextSuccess?'成功':'失败'}`)

const ListPositionArray=[0, 1, 0, 0, 0]
const list=JavaAccessBridge.GetAcessContextByArray(JavaUI,ListPositionArray)
console.log(list)

const SelectedNumber=JavaAccessBridge.GetAccessibleSelectionCountFromContext(list.vmac)
console.log(`已选择数量:${SelectedNumber}`)

const WhetherSelectedFourthSuccess=JavaAccessBridge.AddAccessibleSelectionFromContext(list.vmac,1)
console.log(`选择第四列表项的结果:${WhetherSelectedFourthSuccess?'成功':'失败'}`)

const SelectedNumber2=JavaAccessBridge.GetAccessibleSelectionCountFromContext(list.vmac)
console.log(`已选择数量${SelectedNumber2}`)

const WhetherSelectSuccess=JavaAccessBridge.SelectAllAccessibleSelectionFromContext(list.vmac)
console.log(`选择所有列表项的结果:${WhetherSelectSuccess?'成功':'失败'}`)

const SelectedNumber3=JavaAccessBridge.GetAccessibleSelectionCountFromContext(list.vmac)
console.log(SelectedNumber3)