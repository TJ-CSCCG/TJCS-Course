import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;

public class SaxDemo7 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            factory.setNamespaceAware(true);
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("07.xml", myHandler);
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
        System.out.println("文件中有"+n+"个名称空间。");
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        System.out.print("<"+qName+">");
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        System.out.print("</"+qName+">");
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        System.out.print(new String(ch,start,length));
    }

    public void startPrefixMapping(String prefix, String uri) throws SAXException {
        System.out.println("第"+(++n)+"个名称空间：");
        System.out.println("前缀："+prefix+"名称："+uri);
    }

    public void endPrefixMapping(String prefix) throws SAXException {
        System.out.print("\n前缀为"+prefix+"的名称空间作用域结束。");
    }

}

