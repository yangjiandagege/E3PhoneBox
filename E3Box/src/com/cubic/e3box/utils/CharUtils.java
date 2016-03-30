/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.cubic.e3box.utils;

import java.util.Arrays;

public class CharUtils {
    
	public static boolean isNumeric(String str) {
		for (int i = str.length(); --i >= 0;) {
			if (!Character.isDigit(str.charAt(i))) {
				return false;
			}
		}
		return true;
	}
	
	public static boolean isAlphabetic(String str) {
		for (int i = str.length(); --i >= 0;) {
			if (!Character.isAlphabetic(str.charAt(i))) {
				return false;
			}
		}
		return true;
	}
	
    public static String bytesToHexString(byte[] src){  
        StringBuilder stringBuilder = new StringBuilder("");  
        if (src == null || src.length <= 0) {  
            return null;  
        }  
        for (int i = 0; i < src.length; i++) {  
            int v = src[i] & 0xFF;  
            String hv = Integer.toHexString(v);  
            if (hv.length() < 2) {  
                stringBuilder.append(0);  
            }  
            stringBuilder.append(hv);  
        }  
        return stringBuilder.toString();  
    } 
    
    public static byte[] byteMerger(byte[] byte_1, byte[] byte_2){
    	if(byte_1 == null){
    		return byte_2;
    	}else if(byte_2 == null){
    		return byte_1;
    	}else{
            byte[] byte_3 = new byte[byte_1.length+byte_2.length];  
            System.arraycopy(byte_1, 0, byte_3, 0, byte_1.length);  
            System.arraycopy(byte_2, 0, byte_3, byte_1.length, byte_2.length);  
            return byte_3;
    	}
    }
    
    public static byte[][] bytesSplit(byte[] src,int ARRAY_LENGTH){
    	int arrayNumber = (src.length%ARRAY_LENGTH == 0) ? src.length/ARRAY_LENGTH : src.length/ARRAY_LENGTH + 1;
    	byte[][] array = new byte[arrayNumber][ARRAY_LENGTH];
    	
		for(int i = 0 ; i < arrayNumber ; i++){
			array[i] = Arrays.copyOfRange(src, i*ARRAY_LENGTH, ( (i+1)*ARRAY_LENGTH < src.length )?(i+1)*ARRAY_LENGTH:src.length);
		}
    	return array;
    }
}
