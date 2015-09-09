package vo;

public class RecommendLinks {
	private String id;
	private String algo;
	private String url;
	private String title;
	private String use;
	private String content;
	
	public String getAlgo(){
		return algo;
	}
	public void setAlgo(String algo){
		this.algo = algo;
	}
	
	public String getId(){
		return id;
	}
	public void setId(String id){
		this.id = id;
	}
	
	public String getUrl(){
		return url;
	}
	public void setUrl(String url){
		this.url = url;
	}
	
	public String getTitle(){
		return title;
	}
	public void setTitle(String title){
		this.title = title;
	}
	
	public String getUse(){
		return use;
	}
	public void setUse(String use){
		this.use = use;
	}
	
	public String getContent(){
		return content;
	}
	public void setContent(String content){
		this.content = content;
	}
}
