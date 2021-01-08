import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo1 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("01.xml",myHandler);
        }  catch (Exception e) {
            e.printStackTrace();
        }

    }
}
class MyHandler extends DefaultHandler{
    int n=0;
    public void startDocument() throws SAXException {
        System.out.println("事件"+(++n)+"：startDocument事件，开始解析XML文件");
    }

    public void endDocument() throws SAXException {
        System.out.println("事件"+(++n)+"：endDocument事件，XML文件解析结束");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.println("事件"+(++n)+"：startElement事件");
        System.out.println("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.println("事件"+(++n)+"：endElement事件");
        System.out.println("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.println("事件"+(++n)+"：characters事件");
        String text = new String(ch,start,length);
        System.out.println(text);
    }
}
