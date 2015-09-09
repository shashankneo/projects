package db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.Statement;


public class Sqllitedb {
	public static Sqllitedb dbInstance;
	public static Sqllitedb getInstance() throws Exception {
       
        if (dbInstance == null) {
        	dbInstance = new Sqllitedb();
        	boolean throwException = dbInstance.createTable();
        	if(throwException == false){
        		throw new Exception();
        	}
        }
        return dbInstance;
    }

	public boolean createTable(){
		boolean result = true;
		if(checkTableExists() == true)
			return result;   
		Connection c = null;
	    Statement stmt = null;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      System.out.println("Opened database successfully");

	      stmt = c.createStatement();
	      String sql = "CREATE TABLE USER " +
	                   "(" +
	                   " USERNAME           TEXT    NOT NULL, " + 
	                   " KEYCODE            TEXT     NOT NULL," +
	                    "STATUS            int     NOT NULL)"; 
	      stmt.executeUpdate(sql);
	      stmt.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      return false;
	    }
	    System.out.println("Table created successfully");
		
		return result;
	}
	
	public boolean checkTableExists(){
		boolean result = true;
		Connection c = null;
	    Statement stmt = null;
	    int num = -1;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      stmt = c.createStatement();
	      ResultSet rs = stmt.executeQuery( "SELECT Count(*) as num FROM user;" );
	      rs.close();
	      stmt.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      num = 0;
	    }
	      if( num == 0){
	    	  result = false;
	      }
		return result;
	}
	
	public boolean checkAccountExists(String username){
		boolean result = false;
		Connection c = null;
		PreparedStatement statement = null;
	    int num =0;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "SELECT count(*) as num From user where username=?");
	          statement.setString(1,username);
	        //  statement.setString(2,account_type);
	          ResultSet rs= statement.executeQuery();
	          while ( rs.next() ) {
	 	         num = rs.getInt("num");
	 	      }
	 	      rs.close();
	       
	          c.commit();
	
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	    }
	    if( num == 1){
	    	result = true;
	    }
		return result;
	}
	
	public boolean checkLaptopAccountExists(String username){
		boolean result = false;
		Connection c = null;
		PreparedStatement statement = null;
	    int num =0;
	    int status = 1;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "SELECT count(*) as num, status  From user where username=?");
	          statement.setString(1,username);
	        //  statement.setString(2,account_type);
	          ResultSet rs= statement.executeQuery();
	          while ( rs.next() ) {
	 	         num = rs.getInt("num");
	 	         status = rs.getInt("status");
	 	      }
	 	      rs.close();
	       
	          c.commit();
	
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	    }
	    if( num == 1 && status == 0){
	    	result = true;
	    }
		return result;
	}
	public boolean createAccount(String username, String keycode){
		boolean result = true;
		Connection c = null;
		PreparedStatement statement = null;
	  
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "INSERT INTO USER (USERNAME, KEYCODE, STATUS)"+
	                 " VALUES(?,?,?)");
	          statement.setString(1,username);
	          statement.setString(2,keycode);
	          statement.setInt(3, 0);
	          statement.executeUpdate();
	          c.commit();
	
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      result = false;
	    }
		return result;
	}
	public boolean createLaptopAccount(String username){
		boolean result = true;
		Connection c = null;
		PreparedStatement statement = null;
	  
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "UPDATE USER SET STATUS = 1 where USERNAME = ? ");
	          statement.setString(1,username);
	          statement.executeUpdate();
	          c.commit();
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	      result = false;
	    }
		return result;
	}
	public boolean checkCredentials(String username, String keycode){
		boolean result = false;
		Connection c = null;
		PreparedStatement statement = null;
	    int num =0;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "SELECT count(*) as num from user where username=? and keycode=?");
	          statement.setString(1,username);
	          statement.setString(2,keycode);
	          
	          ResultSet rs= statement.executeQuery();
	          while ( rs.next() ) {
	 	         num = rs.getInt("num");
	 	      }
	 	      rs.close();
	          c.commit();
	
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	    }
	    if( num == 1){
	    	result = true;
	    }
		return result;
	}
	
	public String getKeyCode(String username){
		
		Connection c = null;
		PreparedStatement statement = null;
	    String key = null;
	    try {
	      Class.forName("org.sqlite.JDBC");
	      c = DriverManager.getConnection("jdbc:sqlite:user.db");
	      c.setAutoCommit(false);
	      statement = c.prepareStatement(
	              "SELECT keycode from user where username=?");
	          statement.setString(1,username); 
	          ResultSet rs= statement.executeQuery();
	          while ( rs.next() ) {
	 	         key = rs.getString("keycode");
	 	      }
	 	      rs.close();
	          c.commit();
	
	      statement.close();
	      c.close();
	    } catch ( Exception e ) {
	      System.err.println( e.getClass().getName() + ": " + e.getMessage() );
	    }
	   return key;
	}
}
