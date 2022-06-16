/*
* This file was generated by PolyTaint Truffle instrument - ERO project 2022
*
* The MIT License (MIT)
* Copyright (c) 2022 Peterson Yuhala
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or substantial
* portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
* TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


package polytaint;
import org.graalvm.nativeimage.CurrentIsolate;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;
import org.graalvm.polyglot.*;

public class Untrusted {

    public static Context globalContext = Context.newBuilder().allowAllAccess(true).build();

    public static MultiFunctionOut multiFunc = new MultiFunctionOut(globalContext);

    public static void main(String[] args) {
        System.out.println("Hello from partitioned main!!");
        globalContext.eval("js","function main_wrapper(m){poly_2 = m.poly_2;funcD = m.funcD;funcA = m.funcA;funcN = m.funcN;sayHello = m.sayHello;    return sum;}var reta = funcA(6);sayHello();var retd = funcD(reta) + funcA(2);poly_2();}main_wrapper;").execute(multiFunc);
    }
    public static void poly_2(){
        poly_2_proxy();
    }

    public static int funcA(int param1){
        return funcA_proxy(param1);
    }

    public static int funcD(int param1){
        Param_funcD params = new Param_funcD(param1);
        return globalContext.eval("js","function funcD_wrapper(m,p){poly_2 = m.poly_2;funcA = m.funcA;funcN = m.funcN;sayHello = m.sayHello;param1 = p.param1;function funcD(paramD) {    var res = funcN(paramD, 2);    console.log('funcD res from A: ' + res);    return res;}funcD(param1);}funcD_wrapper;").execute(multiFunc,params).asInt();
    }

    public static int funcN(int param1, int param2){
        Param_funcN params = new Param_funcN(param1, param2);
        return globalContext.eval("js","function funcN_wrapper(m,p){poly_2 = m.poly_2;funcD = m.funcD;funcA = m.funcA;sayHello = m.sayHello;param1 = p.param1;param2 = p.param2;function funcN(param, n) {    console.log('funcN: ' + param);    return param * n;}funcN(param1, param2);}funcN_wrapper;").execute(multiFunc,params).asInt();
    }

    public static void sayHello(){
        globalContext.eval("js","function sayHello_wrapper(m){poly_2 = m.poly_2;funcD = m.funcD;funcA = m.funcA;funcN = m.funcN;function sayHello() {    console.log('++++++++++++ Hello from javascript file +++++++++++');}sayHello();}sayHello_wrapper;").execute(multiFunc);
    }

    @CEntryPoint(name = "funcD_entry")
    public static int funcD_entry(IsolateThread thread, int param1){
        return funcD(param1);
    }

    @CEntryPoint(name = "funcN_entry")
    public static int funcN_entry(IsolateThread thread, int param1, int param2){
        return funcN(param1, param2);
    }

    @CEntryPoint(name = "sayHello_entry")
    public static void sayHello_entry(IsolateThread thread){
        sayHello();
    }

    public static native void poly_2_proxy();
    public static native int funcA_proxy(int param1);
}
