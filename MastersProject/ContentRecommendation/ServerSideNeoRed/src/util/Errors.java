package util;

import network.SomeResponse;

public class Errors {
	public static final String USER_NOT_FOUND = "Username not properly received from client";
	public static final String USER_ALREADY_EXISTS = "User already exists";
	public static final String TRY_AGAIN_LATER = "Server Failed. Try again later";
	
	public static SomeResponse generateError(String error_msg){
		SomeResponse response = new SomeResponse();
		response.command = Constants.errors;
		response.error = error_msg;
		return response;
		
	}
}
