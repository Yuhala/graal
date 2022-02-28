/*
 * Created on Wed Sep 09 2020
 *
 * Copyright (c) 2020 Peterson Yuhala, IIUN
 */
package iiun.smartc;


public class Asset {
    private String assetId;
    private String assetOwner;
    private int assetPrice;

    public Asset(String id, String owner, int price) {
        this.assetId = id;
        this.assetOwner = owner;
        this.assetPrice = price;
        System.out.println("Created asset: "+id);
    }

    public String getAssetId(){
        return assetId;
    }

}
