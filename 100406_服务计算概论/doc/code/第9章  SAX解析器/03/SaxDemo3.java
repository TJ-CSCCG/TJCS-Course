import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo3 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("03.xml",myHandler);
        }  catch (Exception e) {
            e.printStackTrace();
        }

    }
}
class MyHandler extends DefaultHandler{
    long startTime ;
    int n=0;
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
        System.out.println("解析文件所用时间："+(System.currentTimeMillis()-startTime)+"毫秒");
    }

    public void processingInstruction(String target, String data) throws SAXException {
        System.out.println("处理指令事件"+(++n)+":");
        System.out.println("文件中包含的指令"+n+":");
        System.out.println("<?"+target+" "+data+" ?>");
	if(target.equals("displayend")){
		System.out.println("XML文件结束");
	}
    }
}
