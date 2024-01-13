<?xml version="1.0" encoding="gb2312" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/TR/WD-xsl">
  <xsl:template math="/">
	<html>
	<body>
		<center>
		<table border="1" width="300" >
		<tr>
			<td align="center">name</td>
			<td align="center">sex</td>
			<td align="center">age</td>
			<td align="center">lxfs</td>
		</tr>
		<xsl:for-each select="message/persion[age$le$'22']">
		<tr>
			<td align="center"><xsl:value-of select="name" /></td>
			<td align="center"><xsl:value-of select="sex" /></td>
			<td align="center"><xsl:value-of select="age" /></td>
			<td align="center"><xsl:value-of select="lxfs" /></td>
		</tr>
		</xsl:for-each>
		</table>
		</center>
	</body>
	</html>
  </xsl:template>
</xsl:stylesheet>