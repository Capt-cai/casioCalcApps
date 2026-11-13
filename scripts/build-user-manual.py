#!/usr/bin/env python3
"""Render the Chinese user manual with ReportLab and an installed CJK font."""
from pathlib import Path
import argparse, html, re
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.pagesizes import A4
from reportlab.platypus import BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, PageBreak, Table, TableStyle, Image, KeepTogether
from reportlab.platypus.tableofcontents import TableOfContents

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--font', default='/System/Library/Fonts/Supplemental/Arial Unicode.ttf')
parser.add_argument('--output', type=Path, default=ROOT/'output/pdf/ECEN21-Toolkit-User-Manual-zh-CN-M8.pdf')
args = parser.parse_args()
pdfmetrics.registerFont(TTFont('CN', args.font))
pdfmetrics.registerFontFamily('CN', normal='CN', bold='CN', italic='CN', boldItalic='CN')
navy = colors.HexColor('#17354B'); teal = colors.HexColor('#087E8B')
styles = {
    'body': ParagraphStyle('Body', fontName='CN', fontSize=10, leading=16, spaceAfter=6, wordWrap='CJK', splitLongWords=True),
    'h1': ParagraphStyle('Chapter', fontName='CN', fontSize=19, leading=28, textColor=navy, spaceAfter=16, keepWithNext=True),
    'h2': ParagraphStyle('Section', fontName='CN', fontSize=12.5, leading=20, textColor=teal, spaceBefore=10, spaceAfter=7, keepWithNext=True),
    'cell': ParagraphStyle('Cell', fontName='CN', fontSize=9, leading=14, wordWrap='CJK', splitLongWords=True),
    'caption': ParagraphStyle('Caption', fontName='CN', fontSize=9, leading=14, textColor=teal, alignment=TA_CENTER, spaceAfter=10),
    'quote': ParagraphStyle('Note', fontName='CN', fontSize=10, leading=16, textColor=navy, backColor=colors.HexColor('#EDF5F7'), borderPadding=8, spaceBefore=6, spaceAfter=14, wordWrap='CJK'),
}
def inline(s):
    s = html.escape(s)
    s = re.sub(r'`([^`]+)`', r'<font color="#087E8B">\1</font>', s)
    s = re.sub(r'\*\*([^*]+)\*\*', r'<b>\1</b>', s)
    s = re.sub(r'\[([^\]]+)\]\(([^)]+)\)', r'\1', s)
    return s

def footer(canvas, doc):
    if doc.page == 1: return
    canvas.saveState(); canvas.setStrokeColor(teal)
    canvas.line(45, A4[1]-38, A4[0]-45, A4[1]-38)
    canvas.setFont('CN', 8); canvas.setFillColor(navy)
    canvas.drawString(45, A4[1]-29, 'ECEN21 Toolkit  /  中文使用手册')
    canvas.drawRightString(A4[0]-45, 26, f'M8 工程版  |  {doc.page}')
    canvas.restoreState()

class ManualDoc(BaseDocTemplate):
    def afterFlowable(self, flowable):
        if isinstance(flowable, Paragraph) and flowable.getPlainText() != '目录' and flowable.style.name in ('Chapter', 'Section'):
            level = 0 if flowable.style.name == 'Chapter' else 1
            label = flowable.getPlainText(); key = f'section-{self.seq.nextf("heading")}'
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(label, key, level=level, closed=False)
            if level == 0:
                self.notify('TOCEntry', (level, label, self.page, key))

args.output.parent.mkdir(parents=True, exist_ok=True)
doc = ManualDoc(str(args.output), pagesize=A4, leftMargin=45, rightMargin=45, topMargin=54, bottomMargin=46, title='ECEN21 Toolkit 中文使用手册 - M8', author='ECEN21 Toolkit', allowSplitting=True)
doc.addPageTemplates(PageTemplate(id='manual', frames=Frame(45,46,A4[0]-90,A4[1]-100,id='body',leftPadding=0,rightPadding=0,topPadding=0,bottomPadding=0), onPage=footer))
story = [Spacer(1,65), Paragraph('ECEN21<br/>Toolkit', ParagraphStyle('Cover',fontName='CN',fontSize=42,leading=55,textColor=navy)), Spacer(1,25), Paragraph('中文使用手册',ParagraphStyle('CoverSub',fontName='CN',fontSize=25,leading=36,textColor=teal)),Spacer(1,24)]
lines = (ROOT/'docs/USER-MANUAL.zh-CN.md').read_text().splitlines()
for s in lines[2:10]:
    if s.strip(): story.append(Paragraph(inline(s.lstrip('> ')),styles['quote' if s.startswith('>') else 'body']))
story += [Spacer(1,25),Paragraph('七个工具 · 按键说明 · 完整例题 · 文件导出 · 故障处理',styles['body']),PageBreak(),Paragraph('目录',styles['h1'])]
toc = TableOfContents(); toc.levelStyles=[ParagraphStyle('TOC1',fontName='CN',fontSize=10.5,leading=17,spaceBefore=4,textColor=navy),ParagraphStyle('TOC2',fontName='CN',fontSize=9,leading=13,leftIndent=15,textColor=colors.HexColor('#405565'))]
story += [toc]
start = next(i for i,s in enumerate(lines) if s.startswith('## 1.'))
i=start
while i<len(lines):
    line=lines[i].strip()
    if not line: i+=1; continue
    if line.startswith('## '):
        story += [PageBreak(),Paragraph(inline(line[3:]),styles['h1'])]; i+=1
    elif line.startswith('### '):
        story.append(Paragraph(inline(line[4:]),styles['h2'])); i+=1
    elif line.startswith('|'):
        rows=[]
        while i<len(lines) and lines[i].strip().startswith('|'):
            cells=[s.strip() for s in lines[i].strip().strip('|').split('|')]
            if not all(re.fullmatch(r'[:\- ]+',s) for s in cells): rows.append(cells)
            i+=1
        n=max(map(len,rows)); width=A4[0]-90
        ratios={2:[.34,.66],3:[.22,.38,.40],4:[.18,.26,.27,.29],5:[.14,.20,.22,.22,.22]}.get(n,[1/n]*n)
        data=[[Paragraph(inline(c),styles['cell']) for c in row+['']*(n-len(row))] for row in rows]
        table=Table(data,colWidths=[width*r for r in ratios],repeatRows=1,hAlign='LEFT')
        table.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),colors.HexColor('#DDEDF1')),('ROWBACKGROUNDS',(0,1),(-1,-1),[colors.white,colors.HexColor('#F5F8FA')]),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),6),('BOTTOMPADDING',(0,0),(-1,-1),6),('LINEBELOW',(0,0),(-1,0),.5,teal),('LINEBELOW',(0,-1),(-1,-1),.3,colors.HexColor('#C5D5DF'))]))
        story += [KeepTogether([table]) if table.wrap(width, 10000)[1] < 450 else table,Spacer(1,9)]
    elif line.startswith('!['):
        m=re.match(r'!\[([^\]]*)\]\(([^)]+)\)',line)
        image=Image(str(ROOT/'docs'/m[2]),width=380,height=380*448/792)
        story += [image,Paragraph(inline(m[1])+'（真实 M8 模拟器界面）',styles['caption'])]; i+=1
    else:
        is_list=bool(re.match(r'^(\d+\. |[-*] )',line))
        para=[line];i+=1
        if not is_list and not line.startswith('>'):
            while i<len(lines) and lines[i].strip() and not re.match(r'^(#|\||!\[|>|\d+\. |[-*] )',lines[i]):
                para.append(lines[i].strip());i+=1
        text=' '.join(para)
        if re.match(r'^\d+\. ', text):
            steps=[Paragraph(inline(text),styles['body'])]
            while i<len(lines) and re.match(r'^\d+\. ', lines[i]):
                steps.append(Paragraph(inline(lines[i]),styles['body']));i+=1
            height=sum(p.wrap(A4[0]-90,10000)[1]+p.style.spaceAfter for p in steps)
            story.extend([KeepTogether(steps)] if height < 350 else steps)
        else:
            story.append(Paragraph(inline(text.lstrip('> ')),styles['quote' if text.startswith('>') else 'body']))
# Keep short reference sections and their steps together on one page.
packed=[]; cursor=0
while cursor<len(story):
    current=story[cursor]
    if isinstance(current,Paragraph) and current.style.name=='Section':
        end=cursor+1
        while end<len(story) and not isinstance(story[end],PageBreak) and not (isinstance(story[end],Paragraph) and story[end].style.name in ('Chapter','Section')):
            end+=1
        section=[]
        for item in story[cursor:end]:
            section.extend(item._content if isinstance(item,KeepTogether) else [item])
        height=sum(item.wrap(A4[0]-90,10000)[1]+getattr(getattr(item,'style',None),'spaceAfter',0)+getattr(getattr(item,'style',None),'spaceBefore',0) for item in section)
        packed.extend([KeepTogether(section)] if height<450 else story[cursor:end])
        cursor=end
    else:
        packed.append(current);cursor+=1
doc.multiBuild(packed)
print(args.output)
