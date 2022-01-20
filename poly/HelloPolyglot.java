
import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

public class HelloPolyglot {
    public static void main(String[] args) {
        System.out.println("Hello Java!");
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "print('Hello JavaScript!');");
        }

        innerContext();
    }

    static void innerContext() {
        try (Context outer = Context.newBuilder()
                .allowAllAccess(true)
                .build()) {
            outer.eval("js", "inner = Java.type('org.graalvm.polyglot.Context').create()");
            outer.eval("js", "inner.eval('java', System.out.println('Hello inner Java!');");            
            outer.eval("js", "inner.close()");
           
        }
    }
}
