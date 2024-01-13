import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;
import javax.xml.parsers.*;
import java.io.IOException;

public class SaxDemo8 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("08.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    int n=0,m=0;
    public void startDocument() throws SAXException {
        System.out.println("开始解析MXL文件：");
    }

    public void endDocument() throws SAXException {
        System.out.println("解析文件结束");
        System.out.println("解析器报告了"+n+"个实体(包括DOCTYPE声明，"+m+"个被忽略的实体。)");
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

    public InputSource resolveEntity(String publicId, String systemId) throws IOException, SAXException {
        System.out.println("第"+(++n)+"个实体：");
        System.out.println("publicId="+publicId+",systemId="+systemId);
        return null;
    }

    public void skippedEntity(String name) throws SAXException {
        m++;
        System.out.println("第"+(++n)+"个实体,被忽略的实体："+name);
    }

}

