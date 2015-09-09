import scrapy.cmdline

def main():
    scrapy.cmdline.execute(argv=['scrapy', 'crawl', 'zoogle',])


if  __name__ =='__main__':
    main()