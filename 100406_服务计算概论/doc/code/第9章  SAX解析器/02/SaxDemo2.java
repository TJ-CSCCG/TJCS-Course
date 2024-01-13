import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo2 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("02.xml",myHandler);
        }  catch (Exception e) {
            e.printStackTrace();
        }

    }
}
class MyHandler extends DefaultHandler{
    long startTime ; 
    public void startDocument() throws SAXException {
        startTime = System.currentTimeMillis();
        System.out.println("开始解析MXL文件，处理开始事件：");
        System.out.println("可以按照自己的想法写处理方法。");
        System.out.println("开始事件处理完毕\n");
    }

    public void endDocument() throws SAXException {
        System.out.println("处理结束事件：");
        System.out.println("可以按照自己的想法写处理方法。");
        System.out.println("结束事件处理完毕，解析文件结束");
        System.out.println("解析文件所用时间："+(System.currentTimeMillis()-startTime)+"毫秒");
    }
}
