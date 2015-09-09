import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.FileFilter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JEditorPane;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.SwingUtilities;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.text.AttributeSet;
import javax.swing.text.Document;
import javax.swing.text.Element;
import javax.swing.text.html.HTMLEditorKit;
import javax.swing.text.html.StyleSheet;

import vo.RecommendLinks;
import Util.Constants;
import Util.Util;


public class ShowRecommendations {


	private String todayDate;
	private ArrayList<RecommendLinks> recLinks;
	public void showScreen(){
	
			todayDate = findNextFolderToShow();
			recLinks = getLinks();
			if(recLinks != null){
				String body = recommendHtmlBody(recLinks);
				showNotifications(body);
	}
	
			
		}
	private void showNotifications(String bodyText){
		
		SwingUtilities.invokeLater(new Runnable()
	    {
	      public void run()
	      {
	        // create jeditorpane
	        JEditorPane jEditorPane = new JEditorPane();
	        GridBagConstraints constraints = new GridBagConstraints();
	        // make it read-only
	        jEditorPane.setEditable(false);
	        
	        // create a scrollpane; modify its attributes as desired
	        JScrollPane scrollPane = new JScrollPane(jEditorPane);
	        
	        // add an html editor kit
	        HTMLEditorKit kit = new HTMLEditorKit();
	        jEditorPane.setEditorKit(kit);
	        
	        // add some styles to the html
	        StyleSheet styleSheet = kit.getStyleSheet();
	        styleSheet.addRule("body { font-family:times; margin: 4px; }");
	        styleSheet.addRule("h1 {color: blue;}");
	        styleSheet.addRule("h2 {color: #ff0000;}");
	        styleSheet.addRule("pre {font : 10px monaco; color : black; background-color : #fafafa; }");

//	        // create some simple html as a string
//	        String htmlString = "<html>\n"
//	                          + "<body>\n"
//	                          + "<h1>Welcome!</h1>\n"
//	                          + "<h2>This is an H2 header</h2>\n"
//	                          + "<span >This is some sample text</p>\n"
//	                          + "<p><a href=\"http://devdaily.com/blog/\">devdaily blog</a></p>\n"
//	                          + "</body>\n";
	        // create some simple html as a string
	        String htmlString = "<html>\n"
	                          + "<body>\n"
	                          + bodyText 
	                          + "</body>\n";
	        // create a document, set it on the jeditorpane, then add the html
	        Document doc = kit.createDefaultDocument();
	        jEditorPane.setDocument(doc);
	        jEditorPane.setText(htmlString);
	        // now add it all to a frame
	        JFrame j = new JFrame("HtmlEditorKit Test");
	        jEditorPane.addHyperlinkListener(new HyperlinkListener() {
                @Override
                public void hyperlinkUpdate(HyperlinkEvent hle) {
                    if (HyperlinkEvent.EventType.ACTIVATED.equals(hle.getEventType())) {
                    	Element anchor = hle.getSourceElement();
                    	String text = hle.getURL().toString();
                    	String[] texts = text.split(";");
                    	String url = texts[0];
                    	String id = texts[1];
                    	AttributeSet sds = anchor.getAttributes();
                        System.out.println("url="+ url + "id = "+id);
                        writeJson(id, recLinks);
                        recLinks = getLinks();
            			if(recLinks != null){
            				String body = recommendHtmlBody(recLinks);
            				jEditorPane.setText( body);
            			}
                        
                        //showScreen();
                        //Desktop desktop = Desktop.getDesktop();
                        try {
                           // desktop.browse(hle.getURL().toURI());
                        } catch (Exception ex) {
                            ex.printStackTrace();
                        }
                    }
                }
            });
	        
	        
	        j.getContentPane().add(scrollPane, BorderLayout.CENTER);

	        // make it easy to close the application
	        j.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	        
	        // display the frame
	        j.setSize(new Dimension(300,200));
	        
	        

	        // pack it, if you prefer
	        //j.pack();
	        Dimension scrSize = Toolkit.getDefaultToolkit().getScreenSize();// size of the screen
	        Insets toolHeight = Toolkit.getDefaultToolkit().getScreenInsets(j.getGraphicsConfiguration());// height of the task bar
	        j.setLocation(scrSize.width - j.getWidth(), scrSize.height - toolHeight.bottom - j.getHeight());
	      
	        // center the jframe, then make it visible
	       // j.setLocationRelativeTo(null);
	        
	        j.setVisible(true);
	      } 
	    });
	}
	
  private String recommendHtmlBody(ArrayList<RecommendLinks> linksObj){
	  String body = "";
	  for(RecommendLinks linkObj : linksObj){
		  if(linkObj.getUse().equals("yes")){
			  continue;
		  }
		  String html = "";
		  String title = linkObj.getTitle();
		  String content = linkObj.getContent();
		  if(title.length() > Constants.TITLE_MAX){
			  title = title.substring(0,Constants.TITLE_MAX);
		  }
		  if(content.length() > Constants.CONTENT_MAX){
			  content = content.substring(0,Constants.CONTENT_MAX);
		  }
		  String url = linkObj.getUrl() + ";" + linkObj.getId();
		  html = "<div id='"+linkObj.getId()+"'>";
		  html = html + "<p><a href=\'"+url+"'>"+title+"</a></p>\n";
		  html = html + "<p>"+content +"</p>";
		  html = html + "</div>";
		  body = body + html;
	  }
	  return body;
  }
  private ArrayList<RecommendLinks> getLinks(){
	 ArrayList<RecommendLinks> result = null;
	 String dstFolder = System.getProperty("user.dir")+File.separator+Constants.dir_final + File.separator
				+todayDate+ File.separator+ Constants.file_ultimate;
		File f = new File(dstFolder);
		try{
			if(f.exists()){
				String jsonData = Util.readFile(dstFolder);
				if(jsonData != null){
					JSONParser jsonParser=new JSONParser();
					try {
						JSONObject mainObj = (JSONObject)jsonParser.parse(jsonData);
						if(mainObj.containsKey("algo1")){
							JSONArray linksList = (JSONArray)mainObj.get("algo1");
							int i;
							int length = linksList.size();
							if(length>0){
								result = new ArrayList<RecommendLinks>();
							}
							for( i =0;i<length; i++){
								JSONObject linkObj = (JSONObject)linksList.get(i);
								RecommendLinks recObj = new RecommendLinks();
								recObj.setAlgo("algo1");
								recObj.setId((String)linkObj.get("id"));
								recObj.setUse((String)linkObj.get("use"));
								recObj.setContent((String)linkObj.get("content"));
								recObj.setTitle((String)linkObj.get("title"));
								recObj.setUrl((String)linkObj.get("url"));
								result.add(recObj);
							}
						}
					} catch (ParseException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
	 
	 return result;
  }
  private void writeJson(String id, ArrayList<RecommendLinks> recLinks){
	  JSONObject mainObj = new JSONObject();
	  JSONArray linksList = new JSONArray();
	  for(RecommendLinks linkObj : recLinks){
		  JSONObject jsonObj = new JSONObject();
		  if(linkObj.getId().equals(id)){
			  jsonObj.put("use", "yes");
		  }
		  else{
			  jsonObj.put("use", linkObj.getUse());
		  }
		  jsonObj.put("url", linkObj.getUrl());
		  jsonObj.put("content", linkObj.getContent());
		  jsonObj.put("title", linkObj.getTitle());
		  jsonObj.put("id", linkObj.getId());
		  linksList.add(jsonObj);
	  }
	  mainObj.put("algo1",linksList);
	  String dstFolder = System.getProperty("user.dir")+File.separator+Constants.dir_final + File.separator
				+todayDate+ File.separator;
	  Util.writeToFile(mainObj.toJSONString(), dstFolder, Constants.file_ultimate);
  }
  public  String findNextFolderToShow() {

      String lastFolder = null;
      File f = new File(System.getProperty("user.dir") + File.separator + Constants.dir_final);
      File[] files = null;
      //if data folder exists
      if (f.exists()){
          files = f.listFiles(new FileFilter() {
      	    public boolean accept(File file) {
      	        return !file.isHidden();
	      }
	    });
          if(files.length == 0)
            	return lastFolder;
          Arrays.sort(files);
          lastFolder = files[files.length-1].getName();
              
       }
      return lastFolder;
	}
}

