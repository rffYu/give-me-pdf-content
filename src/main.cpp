#include <mupdf/fitz.h>
#include <stdio.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

/*
typedef struct fz_outline
{
        int refs;
        char *title;
        char *uri;
        fz_location page;
        float x, y;
        struct fz_outline *next;
        struct fz_outline *down;
        int is_open;
} fz_outline;
*/

struct TocItem
{
    int level;
    int refs;
    std::string title;

    TocItem(int lvl, int refs, std::string t) : level(lvl), refs(refs), title(t)
    {
    }
};

/*
 def get_toc(
    doc: pymupdf.Document,
    simple: bool = True,
) -> list:
    """Create a table of contents.

    Args:
        simple: a bool to control output. Returns a list, where each entry
consists of outline level, title, page number and link destination (if simple =
False). For details see PyMuPDF's documentation.
    """
    def recurse(olItem, liste, lvl):
        """Recursively follow the outline item chain and record item information
in a list.""" while olItem and olItem.this.m_internal: if olItem.title: title =
olItem.title else: title = " "

            if not olItem.is_external:
                if olItem.uri:
                    if olItem.page == -1:
                        resolve = doc.resolve_link(olItem.uri)
                        page = resolve[0] + 1
                    else:
                        page = olItem.page + 1
                else:
                    page = -1
            else:
                page = -1

            if not simple:
                link = getLinkDict(olItem, doc)
                liste.append([lvl, title, page, link])
            else:
                liste.append([lvl, title, page])

            if olItem.down:
                liste = recurse(olItem.down, liste, lvl + 1)
            olItem = olItem.next
        return liste

    # ensure document is open
    if doc.is_closed:
        raise ValueError("document closed")
    doc.init_doc()
    olItem = doc.outline
    if not olItem:
        return []
    lvl = 1
    liste = []
    toc = recurse(olItem, liste, lvl)
    if doc.is_pdf and simple is False:
        doc._extend_toc_items(toc)
    return toc
*/

std::vector<std::unique_ptr<TocItem>> get_toc(fz_context* ctx, fz_document* doc)
{
    fz_outline* olItem = fz_load_outline(ctx, doc);
    if (!olItem)
    {
        std::cout << "No toc found" << std::endl;
        return {};
    }

    std::vector<std::unique_ptr<TocItem>> toc;

    std::function<void(fz_outline*, int)> recurse =
        [&](fz_outline* item, int level)
    {
        while (item)
        {
            std::string title = item->title ? item->title : " ";
            int page = -1;

            if (item->uri)
            {
                if (item->page.page == -1)
                {
                    int resolve_page;
                    fz_document* resolve_doc;
                    // fz_resolve_link(ctx, doc, item->uri, &resolve_doc,
                    //                 &resolve_page);
                    page = resolve_page + 1;
                }
                else
                {
                    page = item->page.page + 1;
                }
            }
            toc.push_back(std::make_unique<TocItem>(level, page, title));

            if (item->down)
            {
                recurse(item->down, level + 1);
            }
            item = item->next;
        }
    };
    // end of recurse

    recurse(olItem, 1);
    return toc;
}

std::unique_ptr<fz_context, decltype(&fz_drop_context)> createContext()
{
    fz_context* ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    if (!ctx)
    {
        throw std::runtime_error("Failed to create MuPDF context");
    }
    return std::unique_ptr<fz_context, decltype(&fz_drop_context)>(
        ctx, &fz_drop_context);
}

fz_document* get_pdf_document(fz_context* ctx, const std::string& filename)
{
    fz_document* doc;
    fz_try(ctx)
    {
        doc = fz_open_document(ctx, filename.c_str());
        if (!doc)
        {
            throw std::runtime_error("Failed to open PDF document");
        }
    }
    fz_catch(ctx)
    {
        fz_report_error(ctx);
        std::cerr << "cannot open document" << std::endl;
        fz_drop_context(ctx);
        throw std::runtime_error("Failed to open PDF file");
    }
    return doc;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    if (filename == "")
    {
        throw std::runtime_error("Invalid arg");
    }

    std::unique_ptr<fz_context, decltype(&fz_drop_context)> ctx =
        createContext();

    fz_register_document_handlers(ctx.get());

    auto doc = get_pdf_document(ctx.get(), filename);
    std::cout << "Open document finished" << std::endl;
    auto toc = get_toc(ctx.get(), doc);

    // clean up
    fz_drop_document(ctx.get(), doc);

    std::cout << "Here come the content:\n"
              << std::string(48, '=') << std::endl;
    for (const auto& t : toc)
    {
        std::cout << t->level << " " << t->title << " " << t->refs << std::endl;
    }
    return 0;
}
