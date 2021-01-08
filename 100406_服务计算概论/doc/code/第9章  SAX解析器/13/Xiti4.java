import org.xml.sax.helpers.DefaultHandler;
import org.xml.sax.*;

import javax.xml.parsers.*;

public class Xiti4 {
    public static void main(String[] args) {
        try {
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParser = factory.newSAXParser();
            MyHandler myHandler = new MyHandler();
            saxParser.parse("xiti4.xml", myHandler);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }
}

class MyHandler extends DefaultHandler {
    float math = 0, english = 0;
    float m = 0, en = 0;
    int n=0;
    boolean flag = false, fmath = false, feng = false;

    public void startDocument() throws SAXException {
        System.out.print("=====成绩单=====");
    }

    public void endDocument() throws SAXException {
        System.out.println("=====平均分=====");
        System.out.println("数学："+(math/n));
        System.out.println("英语："+(english/n));
    }

    public void startElement(String uri, String localName, String qName, Attributes attributes) throws SAXException {
        if (qName.equals("name")) {
            flag = true;
            n++;
        }
        if (qName.equals("math")) {
            fmath = true;
        }
        if (qName.equals("english")) {
            feng = true;
        }
    }

    public void endElement(String uri, String localName, String qName) throws SAXException {
        if (qName.equals("name")) {
            System.out.println("总分:" + (m + en));
        }
    }

    public void characters(char ch[], int start, int length) throws SAXException {
        if (flag) {
            System.out.println(new String(ch, start, length));
            flag = false;
        }
        if (fmath) {
            m = Float.parseFloat(new String(ch, start, length));
            System.out.println("数学：" + m);
            math += m;
            fmath = false;
        }
        if (feng) {
            en = Float.parseFloat(new String(ch, start, length));
            System.out.println("英语：" + en);
            english += en;
            feng = false;
        }
    }
}



