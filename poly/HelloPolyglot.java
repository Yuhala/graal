
import org.graalvm.polyglot.*;
import org.graalvm.polyglot.proxy.*;

public class HelloPolyglot {
    public static void main(String[] args) {
        System.out.println("Hello Java!");
        // context without try block
        Context enclave = Context.create();
        Value array = enclave.eval("js", "[1,2,42,4]");
        int result = array.getArrayElement(2).asInt();
        System.out.println("Result is: " + result);

        int myInt = enclave.eval("js", "2").asInt();
        System.out.println("MyInt is: " + myInt);

        enclave.eval("js", "print('Hello enclave!');");
    }

    static void contextWithTry() {
        try (Context context = Context.create()) {
            System.out.println("About to run js context");
            context.eval("js", "print('Hello JavaScript!');");

        }

    }
}
