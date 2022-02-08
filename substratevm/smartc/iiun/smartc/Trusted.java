
/*
 * Created on Sun May 16 2021
 *
 * Copyright (c) 2021 Peterson Yuhala, IIUN
 */


package iiun.smartc;

public class Trusted {
    private int id;

    //random string name
    private String name;

    public Trusted(int n){
        this.id = n;
        this.name = getRandString(32);
    }


    // https://www.geeksforgeeks.org/generate-random-string-of-given-size-in-java/
    public static String getRandString(int length) {
        // chose a Character random from this String
        String AlphaNumericString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" + "0123456789" + "abcdefghijklmnopqrstuvxyz";

        // create StringBuffer size of AlphaNumericString
        StringBuilder sb = new StringBuilder(length);

        for (int i = 0; i < length; i++) {
            // generate a random number between
            // 0 to AlphaNumericString variable length
            int index = (int) (AlphaNumericString.length() * Math.random());

            // add Character one by one in end of sb
            sb.append(AlphaNumericString.charAt(index));
        }

        return sb.toString();
    }

}
