/*
 * Created on Tue Mar 01 2022
 *
 * The MIT License (MIT) Copyright (c) 2022 Peterson Yuhala
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 * NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

package poly;

import org.graalvm.polyglot.*;
import org.graalvm.nativeimage.SGXSerializer;
import java.util.List;


public class Polyglot {
    Context context = Context.newBuilder().allowAllAccess(true).build();
    public static final TypeLiteral<List<List<Double>>> DOUBLE_LIST =
            new TypeLiteral<List<List<Double>>>() {};

    public static final TypeLiteral<List<Double>> SIMPLE_LIST = new TypeLiteral<List<Double>>() {};

    public static void main(String[] args) {
        // Context context = Context.newBuilder().allowAllAccess(true).build();
        // Value js_multi = context.eval("js", "(function multi(a, b) {return a * b;})");
        // Value res = js_multi.execute(6, 7);
        // System.out.println(res);

        // Value java_hello =
        // context.asValue(Polyglot.class).getMember("static").getMember("javaHello");
        // Value js_hello = context.eval("js", "function
        // jsHello(java_hello){java_hello();}jsHello;");
        // js_hello.execute(java_hello);

        polyObjects();

    }

    /**
     * Test exchange of object
     */
    public static void polyObjects() {
        Context context = Context.newBuilder().allowAllAccess(true).build();

        MyArray marray = new MyArray(3);



        //@formatter:off
        Value val = context.eval("js", "(function getArray(a) {return [1.1,2.2,3.4];})").execute(marray);
        List<Double> list1 = context.eval("js", "(function getArray() {return [1.1,2.2];})").execute().as(SIMPLE_LIST);
        Value res = context.eval("js", "(function getArray() {return [[1.1,2.2],[3.3,4.4,5.5]];})").execute();
        
        List<List<Double>> list2 = res.as(DOUBLE_LIST);
        System.out.println("List elements from JS: "+list2.toString());
        System.out.println("List1 elements: "+list1.toString());
        //@formatter:on



        // long size = array.getArraySize();
        // System.out.println("array[2] is: " + marray.array[2]);

        Double[][] doubleArray =
                list2.stream().map(l -> l.toArray(new Double[l.size()])).toArray(Double[][]::new);
        Double[] simple = list1.toArray(new Double[0]);


        // javaList.toArray(doubleArray);
        System.out.println("Double[] simple elt 0: " + simple[0]);

        System.out.println("DoubleArray[1][1] = " + doubleArray[1][1]);
        Value param = Value.asValue(doubleArray);

        Value val2 = context
                .eval("js", "(function getArray(dd) {console.log(\"dd[1][2] = \",dd[1][2]);})")
                .execute(param);

        int[] darray;
        // System.out.println(darray[0]);
        int[] array = {456, 22, 33, 44};
        byte[] tempBuf = new byte[256 * 1024 * 1024];

        System.out.println("Temp buffer length = " + tempBuf.length);

        byte[] bytes = SGXSerializer.serialize(doubleArray);
        int len = bytes.length;
        System.out.println("serialized bytes: " + bytes.toString());

        SGXSerializer.arrayCopy(tempBuf, bytes, len);

        // Object desArray = SGXSerializer.deserialize(tempBuf);
        Object objList = SGXSerializer.deserialize(tempBuf);
        Double[][] desList = (Double[][]) objList;

        System.out.println("Deserialized list [1][2]: " + desList[1][2]);

        // Value arrayValue = Value.asValue(desArray);


        // Value printJavaArray = context.eval("js",
        // "(function printJavaArray(o) {console.log(\"on-enter-printJavaArray\");var val =
        // o[0];console.log(\"A[0] = \", val);})");
        // printJavaArray.execute(arrayValue);
    }


    public static void javaHello() {
        System.out.println("Hello Java");
    }

    static void contextWithTry() {
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "print('Hello JavaScript!');");

        }

    }

    /**
     * Exporting js function/method as Java method.
     */
    public static void testJsFunction() {

        Context context = Context.create();
        Source src_multi =
                Source.newBuilder("js", "(function multi(a, b) {return a * b;})", "mul.js")
                        .buildLiteral();

        Value js_multi = context.eval(src_multi);


        Value res = js_multi.execute(6, 7);
        System.out.println("Result is: " + res);


    }

    public static void helloJs() {


        Context ctx = Context.create();
        // run javascript code
        ctx.eval("js", "print('Hello javascript!');");
        Value array = ctx.eval("js", "[1,2,42,4]");

        int result = array.getArrayElement(2).asInt();
        System.out.println("Result is: " + result);

        int myInt = ctx.eval("js", "2").asInt();
        System.out.println("My int in JS is: " + myInt);

    }

    public static void testSl() {
        Context ctx = Context.create();
        // run simple language code
        ctx.eval("secureL", "function main() { println(\"Hello secure language!\");}");
        ctx.eval("secureL", "function main() { println(10/3);}");

        int secureInt = ctx.eval("secureL", "function main() {return 50;}").asInt();
        System.out.println("My int in SL is: " + secureInt);
    }

}

/*
 * #include <stdio.h> #include <graalvm/llvm/polyglot.h>
 * 
 * int main() { void *array = polyglot_eval("js", "[1,2,42,4]"); int element =
 * polyglot_as_i32(polyglot_get_array_element(array, 2)); printf("%d\n", element); return element; }
 */
