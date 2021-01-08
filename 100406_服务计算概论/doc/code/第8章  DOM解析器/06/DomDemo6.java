import org.w3c.dom.*;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import java.io.File;

public class DomDemo6 {
     public static void main(String[] args) {
         int m=1;
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            Document document = builder.parse(new File("05.xml"));

            DocumentType docType = document.getDoctype();
            System.out.println("根标记名称："+docType.getName());
            System.out.println("正式公用标识符："+docType.getPublicId());
            System.out.println("关联的外部DTD："+docType.getSystemId());
            System.out.println("文件内部的DTD：\n"+docType.getInternalSubset());
            } catch (Exception e) {
            e.printStackTrace();
        }

    }



}
