import org.w3c.dom.*;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.Transformer;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;
import java.io.File;
import java.io.FileOutputStream;

public class DomDemo9 {
     public static void main(String[] args) {
         String names[] ={"小张","小王","小李"};
         String tels[] = {"1111111","2222222","3333333"};
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.newDocument();
            document.setXmlVersion("1.0");
            Element root = document.createElement("message");
            document.appendChild(root);

            for(int i=0;i<names.length;i++){
            Element persion = document.createElement("persion");
            persion.setAttribute("id","00"+(i+1));
            root.appendChild(persion);

            Element name = document.createElement("name");
            name.appendChild(document.createTextNode(names[i]));
            persion.appendChild(name);

            Element tel = document.createElement("tel");
            tel.appendChild(document.createTextNode(tels[i]));
            persion.appendChild(tel);
            }

            TransformerFactory tfactory =  TransformerFactory.newInstance();
            Transformer transformer = tfactory.newTransformer();
            DOMSource source = new DOMSource(document);
            FileOutputStream out = new FileOutputStream(new File("message.xml"));
            StreamResult result = new StreamResult(out);
            transformer.transform(source, result);
        }catch(Exception e){
            e.printStackTrace();
        }
    }
}
