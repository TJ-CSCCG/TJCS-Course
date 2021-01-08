import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;

import javax.xml.parsers.*;

public class SaxDemo5 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("05.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    boolean content = false;
    int m=0;
    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
        System.out.println("共处理了"+m+"个characters事件。");

    }

    public void characters(char ch[], int start, int length) throws SAXException {
        m++;
        if(content){
            System.out.println(new String(ch,start,length));
            content = false;
        }
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        if(localName.equals("title")){
            content = true;
            System.out.print("书名：");
        }
        if(localName.equals("author")){
            content = true;
            System.out.print("作者：");
        }
    }
      
}

