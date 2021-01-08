import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;

import javax.xml.parsers.*;

public class SaxDemo6 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("06.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    int n=0;
    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
        System.out.println("共处理了"+n+"次空白。");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println("<"+localName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("</"+localName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.println(new String(ch,start,length));
    }

    public void ignorableWhitespace(char ch[], int start, int length) throws SAXException {
        System.out.print("空白"+(++n));
    }

}

