/*
 * Created on Wed Feb 10 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 * 
 */

package org.graalvm.nativeimage;

/**
 * We use this class to serialize/deserialize parameter or return values for non object types used
 * in relay methods of SGX native images. We try to avoid accessing classes reflectively. We track
 * the types to be serialized during bytecode transformation and call the serializer methods
 * appropriately with the correct types.
 */

import java.io.*;
import java.util.Arrays;
import java.nio.charset.StandardCharsets;

import org.graalvm.nativeimage.PinnedObject;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CIntPointer;
import org.graalvm.nativeimage.c.type.CLongPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;
import org.graalvm.nativeimage.c.type.CTypeConversion.CCharPointerHolder;
import org.graalvm.word.Pointer;
import org.graalvm.word.PointerBase;
import org.graalvm.word.UnsignedWord;
import org.graalvm.word.WordFactory;

public class SGXSerializer {

    /**
     * Serializes an object it into a byte array.
     * 
     * @param o
     * @return
     * @throws Exception
     */
    public static byte[] serialize(Object o) {

        byte[] bytes = null;
        ByteArrayOutputStream bos = null;
        ObjectOutputStream out = null;

        try {
            bos = new ByteArrayOutputStream();
            out = new ObjectOutputStream(bos);
            out.writeObject(o);
            out.flush();
            bytes = bos.toByteArray();

        } catch (IOException e) {
            e.getStackTrace();

        } finally {
            try {
                bos.close();
            } catch (IOException e) {
                // do nothing

            }
        }

        return bytes;
    }

    /**
     * Deserializes a byte array into a generic object. The object will be casted to the appropriate
     * type at the consumer end.
     * 
     * @param bytes
     * @return
     * @throws Exception
     */

    public static Object deserialize(byte[] bytes) {

        ByteArrayInputStream bis = null;
        ObjectInput in = null;
        Object o = null;
        try {
            bis = new ByteArrayInputStream(bytes);
            in = new ObjectInputStream(bis);
            o = in.readObject();
        } catch (IOException e) {
            e.getStackTrace();

        } catch (ClassNotFoundException e) {
            e.getStackTrace();
        } finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (IOException e) {
                // do nothing
                e.getStackTrace();

            }
        }

        return o;
    }

    /**
     * Returns the address of the byte array as a graal CCharPointer. This value will be passed as
     * param in the relay method for the corresponding object. It is possible to do the
     * serialization here directly but choose not to because we will require the length of the
     * latter to pass to relay methods. Using pinned objects prevents the GC from moving the object
     * which may lead to errors when we try to copy from an address for a non-existent object or
     * invalid pointer.
     * 
     * @param bytes
     * @return
     */
    public static CCharPointer getCharPointer(byte[] bytes) throws Exception {

        try (PinnedObject pinnedBuffer = PinnedObject.create(bytes)) {
            CCharPointer charPtr = pinnedBuffer.addressOfArrayElement(0);
            return charPtr;

        } catch (Exception e) {
            e.getStackTrace();
        }

        /*
         * String byteString = new String(bytes, StandardCharsets.UTF_8); CCharPointerHolder pin =
         * CTypeConversion.toCString(byteString);
         * 
         * final CCharPointer charPtr = pin.get(); return charPtr;
         */
        System.out.println("CharPointer null");
        return WordFactory.nullPointer();
    }

    /**
     * Constructs a byte buffer corresponding to the CCharPointer and length.
     * 
     * @param charPtr
     * @param len
     * @return
     */
    public static byte[] getByteBuffer(CCharPointer charPtr, int len) {
        byte[] bytes = new byte[len];
        for (int i = 0; i < bytes.length; i++) {
            bytes[i] = ((Pointer) charPtr).readByte(i);
        }
        return bytes;
    }

    /**
     * Copies src byte array into destination byte array, byte by byte.
     * 
     * @param dest
     * @param src
     */
    public static void arrayCopy(byte[] dest, byte[] src, int size) {
        for (int i = 0; i < size; i++) {
            dest[i] = src[i];
        }
    }

    /**
     * Fills the input array with 0x0 starting from specified index
     * 
     * @param array
     * @param startIdx
     */
    public static void zeroFill(byte[] array, int startIdx) {

        for (int i = startIdx; i < array.length; i++) {
            array[i] = 0x0;
        }
    }

}

