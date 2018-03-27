#include <htmlcxx/html/ParserDom.h>
#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <set>
#include <cmath>

using namespace std;
using namespace htmlcxx;

string get_current_time_str()
{
    time_t cur = time(NULL);
    struct tm *ptm = localtime(&cur);
    char buf[64];
    snprintf(buf, sizeof(buf), "%04d/%02d/%02d %02d:%02d:%02d",
             ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return buf;
}

enum class CellStatus
{
    EMPTY, PARTIAL, FULL
};

class CellId
{
    string user;
    string problem;

public:
    CellId(const string &user_, const string &problem_) : user(user_), problem(problem_) {}
    const string &get_user() const { return user; }
    const string &get_problem() const { return problem; }

    int compare(const CellId &ci2) const
    {
        int r = user.compare(ci2.user);
        if (r != 0) return r;
        return problem.compare(ci2.problem);
    }

    friend bool operator == (const CellId &ci1, const CellId &ci2)
    {
        return ci1.user == ci2.user && ci1.problem == ci2.problem;
    }
    friend bool operator != (const CellId &ci1, const CellId &ci2)
    {
        return ci1.user != ci2.user || ci1.problem != ci2.problem;
    }
    friend bool operator < (const CellId &ci1, const CellId &ci2)
    {
        return ci1.compare(ci2) < 0;
    }
    friend bool operator <= (const CellId &ci1, const CellId &ci2)
    {
        return ci1.compare(ci2) <= 0;
    }
    friend bool operator > (const CellId &ci1, const CellId &ci2)
    {
        return ci1.compare(ci2) > 0;
    }
    friend bool operator >= (const CellId &ci1, const CellId &ci2)
    {
        return ci1.compare(ci2) >= 0;
    }
};

class Cell
{
    CellStatus status = CellStatus::EMPTY;
    int score = 0;

public:
    Cell() {}
    Cell(CellStatus status_, int score_) : status(status_), score(score_) {}
    CellStatus get_status() const { return status; }
    int get_score() const { return score; }
};

class GroupInfo
{
    string name;
    string file;

    int user_count = 0;

    vector<int> places;
    vector<int> scores;
    vector<int> problems;

    bool place_avg_computed = false;
    double place_avg = 0.0;

    bool score_avg_computed = false;
    double score_avg = 0.0;

    bool problem_avg_computed = false;
    double problem_avg = 0.0;

    bool place_s_computed = false;
    double place_s = 0.0;

    bool score_s_computed = false;
    double score_s = 0.0;

    bool problem_s_computed = false;
    double problem_s = 0.0;

public:
    GroupInfo(const string &name_, const string &file_) : name(name_), file(file_) {}
    const string &get_name() const { return name; }
    const string &get_file() const { return file; }

    void add_stat(int place, int score, int problem)
    {
        ++user_count;
        places.push_back(place);
        scores.push_back(score);
        problems.push_back(problem);
    }

    int get_user_count() const { return user_count; }

    static double get_avg(const vector<int> &v)
    {
        if (v.size() <= 0) return 0.0;
        double s = 0.0;
        for (int p : v) {
            s += p;
        }
        return s / v.size();
    }
    double get_place_avg()
    {
        if (!place_avg_computed) {
            place_avg = get_avg(places);
            place_avg_computed = true;
        }
        return place_avg;
    }
    string get_place_avg_str()
    {
        if (user_count <= 0) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_place_avg());
        return buf;
    }
    double get_score_avg()
    {
        if (!score_avg_computed) {
            score_avg = get_avg(scores);
            score_avg_computed = true;
        }
        return score_avg;
    }
    string get_score_avg_str()
    {
        if (user_count <= 0) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_score_avg());
        return buf;
    }
    double get_problem_avg()
    {
        if (!problem_avg_computed) {
            problem_avg = get_avg(problems);
            problem_avg_computed = true;
        }
        return problem_avg;
    }
    string get_problem_avg_str()
    {
        if (user_count <= 0) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_problem_avg());
        return buf;
    }

    static double get_s(const vector<int> &v, double avg)
    {
        if (v.size() <= 1) return 0.0;
        double s = 0.0;
        for (int p : v) {
            s += (p - avg) * (p - avg);
        }
        return sqrt(s / (v.size() - 1));
    }

    double get_place_s()
    {
        if (!place_s_computed) {
            place_s = get_s(places, get_place_avg());
            place_s_computed = true;
        }
        return place_s;
    }
    string get_place_s_str()
    {
        if (user_count <= 1) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_place_s());
        return buf;
    }
    double get_score_s()
    {
        if (!score_s_computed) {
            score_s = get_s(scores, get_score_avg());
            score_s_computed = true;
        }
        return score_s;
    }
    string get_score_s_str()
    {
        if (user_count <= 1) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_score_s());
        return buf;
    }
    double get_problem_s()
    {
        if (!problem_s_computed) {
            problem_s = get_s(problems, get_problem_avg());
            problem_s_computed = true;
        }
        return problem_s;
    }
    string get_problem_s_str()
    {
        if (user_count <= 1) return "N/A";
        char buf[64];
        sprintf(buf, "%.2f", get_problem_s());
        return buf;
    }

    static double get_mediana(const vector<int> &v)
    {
        if (v.size() <= 0) {
            return 0.0;
        } else if (v.size() % 2 == 0) {
            return (v[v.size() / 2 - 1] + v[v.size() / 2] + 0.0) / 2;
        } else {
            return v[v.size() / 2];
        }
    }
    double get_place_mediana() const
    {
        return get_mediana(places);
    }
    string get_place_mediana_str() const
    {
        char buf[64];
        sprintf(buf, "%.2f", get_place_mediana());
        return buf;        
    }
    double get_score_mediana() const
    {
        return get_mediana(scores);
    }
    string get_score_mediana_str() const
    {
        char buf[64];
        sprintf(buf, "%.2f", get_score_mediana());
        return buf;        
    }
    double get_problem_mediana() const
    {
        return get_mediana(problems);
    }
    string get_problem_mediana_str() const
    {
        char buf[64];
        sprintf(buf, "%.2f", get_problem_mediana());
        return buf;        
    }
};

class ProblemInfo
{
    string name;
    int score = 0;
    string category;
    int column = -1;

public:
    ProblemInfo(const string &name_, int score_, const string &category_) : name(name_), score(score_), category(category_) {}
    const string &get_name() const { return name; }
    int get_score() const { return score; }
    const string &get_category() const { return category; }

    void set_column(int column) { this->column = column; }
    int get_column() const { return column; }
};

struct CategorySpec
{
    string name;
    bool crediting;
    string grader;

    CategorySpec(const string &name_, bool crediting_, const string &grader_) : name(name_), crediting(crediting_), grader(grader_) {}
};

struct CategoryInfo
{
    int index = 0;
    bool crediting = false;
    string grader;
    int count = 0;
    int current = 0;
    int start_pos = 0;
    int max_score = 0;

    CategoryInfo(int index_, bool crediting_, const string &grader_) : index(index_), crediting(crediting_), grader(grader_) {}
};

struct UserInfo
{
    string name;
    string group;
    vector<Cell> row;
    vector<int> score_by_cat;
    vector<int> prob_by_cat;
    vector<int> score_by_grad;
    vector<int> prob_by_grad;
    vector<int> mark_by_grad;

    int total_score = 0;
    int total_prob = 0;

public:
    UserInfo(const string &name_, const string &group_, int count, int cat_count, int grad_count)
        : name(name_), group(group_), row(count),
          score_by_cat(cat_count), prob_by_cat(cat_count),
          score_by_grad(grad_count), prob_by_grad(grad_count), mark_by_grad(grad_count, -1)
    {
    }
};

struct GradeInfo
{
    string name;
    int mode = 0;
    int marks[101];
    int max_score = 0;
    int prob_count = 0;

public:
    GradeInfo(const string &name_, int mode_) : name(name_), mode(mode_)
    {
        memset(marks, -1, sizeof(marks));
    }
};

class SortByScore;
class SortByProblems;

class Course
{
    vector<GroupInfo> groups;
    GroupInfo group_all{"All", ""};
    map<string, int> groupidx;
    map<string, ProblemInfo> problems;
    map<string, string> usergroups;
    map<string, set<string> > usergrsets;
    map<CellId, Cell> cells;
    vector<CategorySpec> categories;
    map<string, CategoryInfo> catinfos;
    map<string, UserInfo> userinfos;
    int problem_count = 0;
    vector<GradeInfo> grades;
    map<string, int> grade_idx;
    int sort_mode = 0;
    bool hide_summary = false;
    bool show_problems = false;
    bool show_accumulated = false;
    bool hide_marks = false;
    bool hide_grades = false;
    bool show_percent = false;
    string footer_name;
    string header_name;
    string notes_name;
    int max_score = 0;

public:
    void add_group(const string &name, const string &file)
    {
        if (groupidx.find(name) != groupidx.end()) return;
        groups.push_back(GroupInfo(name, file));
        groupidx.insert(make_pair(name, int(groups.size() - 1)));
    }
    void add_problem(const string &name, int score, const string &category)
    {
        problems.insert(make_pair(name, ProblemInfo(name, score, category)));
        max_score += score;
    }

    bool parse_config(const char *path);
    bool process_group(const GroupInfo &gi);
    bool process_groups()
    {
        bool result = true;
        for (const auto &gi : groups) {
            result = process_group(gi) && result;
        }

        /*
        for (const auto &v : cells) {
            cout << v.first.get_user() << ", " << v.first.get_problem() << " -> " << v.second.get_score() << endl;
        }
        */

        return result;
    }
    void assign_columns();
    void assign_users();

    friend class SortByScore;
    friend class SortByProblems;
};

string read_file(const std::string &path)
{
    string buf;
    int c;

    FILE *f = fopen(path.c_str(), "r");
    if (!f) {
        fprintf(stderr, "cannot open file '%s'\n", path.c_str());
        exit(1);
    }
    while ((c = getc(f)) != EOF) {
        buf.push_back(char(c));
    }
    return buf;
}

bool Course::parse_config(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "cannot open config file '%s'\n", path);
        return false;
    }
    char buf[1024];
    while (fgets(buf, sizeof(buf), f)) {
        int len = strlen(buf);
        while (len > 0 && isspace(buf[len - 1])) --len;
        buf[len] = 0;
        if (len <= 0) continue;

        char cmd[1024];
        int n;
        if (sscanf(buf, "%s%n", cmd, &n) != 1) {
            fprintf(stderr, "invalid line '%s'\n", buf);
            continue;
        }
        if (!strcmp(cmd, "hide_summary")) {
            hide_summary = true;
        } else if (!strcmp(cmd, "hide_marks")) {
            hide_marks = true;
        } else if (!strcmp(cmd, "hide_grades")) {
            hide_grades = true;
        } else if (!strcmp(cmd, "show_percent")) {
            show_percent = true;
        } else if (!strcmp(cmd, "show_problems")) {
            show_problems = true;
        } else if (!strcmp(cmd, "show_accumulated")) {
            show_accumulated = true;
        } else if (!strcmp(cmd, "header")) {
            char ffile[1024];
            if (sscanf(buf, "%s%s%n", cmd, ffile, &n) != 2 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            header_name.assign(ffile);
        } else if (!strcmp(cmd, "notes")) {
            char ffile[1024];
            if (sscanf(buf, "%s%s%n", cmd, ffile, &n) != 2 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            notes_name.assign(ffile);
        } else if (!strcmp(cmd, "footer")) {
            char ffile[1024];
            if (sscanf(buf, "%s%s%n", cmd, ffile, &n) != 2 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            footer_name.assign(ffile);
        } else if (!strcmp(cmd, "group")) {
            char gname[1024];
            char gfile[1024];
            if (sscanf(buf, "%s%s%s%n", cmd, gname, gfile, &n) != 3 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            add_group(gname, gfile);
        } else if (!strcmp(cmd, "problem")) {
            char pname[1024];
            int pscore = -1;
            char pcategory[1024];
            if (sscanf(buf, "%s%s%d%s%n", cmd, pname, &pscore, pcategory, &n) != 4 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            add_problem(pname, pscore, pcategory);
        } else if (!strcmp(cmd, "category")) {
            char cname[1024];
            int crediting;
            char cgrader[1024];
            if (sscanf(buf, "%s%s%d%s%n", cmd, cname, &crediting, cgrader, &n) != 4 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            categories.push_back(CategorySpec(cname, crediting, cgrader));
        } else if (!strcmp(cmd, "grade")) {
            char gname[1024];
            int gmode;
            int gperc;
            int gmark;
            if (sscanf(buf, "%s%s%d%d%d%n", cmd, gname, &gmode, &gperc, &gmark, &n) != 5 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            if (gperc < 0 || gperc > 100) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            auto gii = grade_idx.find(gname);
            if (gii == grade_idx.end()) {
                grades.push_back(GradeInfo(gname, gmode));
                grade_idx.insert(make_pair(gname, int(grades.size() - 1)));
                gii = grade_idx.find(gname);
                if (gii == grade_idx.end()) abort();
            }
            GradeInfo &gi = grades[gii->second];
            for (int i = gperc; i <= 100; ++i) {
                if (gi.marks[i] < 0) gi.marks[i] = gmark;
            }
        } else if (!strcmp(cmd, "sort")) {
            int sort_mode = 0;
            if (sscanf(buf, "%s%d%n", cmd, &sort_mode, &n) != 2 || buf[n]) {
                fprintf(stderr, "invalid line '%s'\n", buf);
                continue;
            }
            this->sort_mode = sort_mode;
        } else {
            fprintf(stderr, "invalid line '%s'\n", buf);
        }
    }
    fclose(f);
    return true;
}

bool Course::process_group(const GroupInfo &gi)
{
    string html = read_file(gi.get_file());
    HTML::ParserDom parser;
    tree<HTML::Node> dom = parser.parseTree(html);

    auto root = dom.begin();
    auto html_node = dom.end();
    for (auto i = dom.begin(root); i != dom.end(root); ++i) {
        if (i->isTag() && i->tagName() == "html") {
            html_node = i;
        }
    }
    /*
    if (html_node != dom.end()) {
        cout << *html_node << endl;
    }
    */
    auto body_node = dom.end();
    for (auto i = dom.begin(html_node); i != dom.end(html_node); ++i) {
        if (i->isTag() && i->tagName() == "body") {
            body_node = i;
        }
    }
    /*
    if (body_node != dom.end()) {
        cout << *body_node << endl;
    }
    */
    auto table_node = dom.end();
    for (auto i = dom.begin(body_node); i != dom.end(body_node); ++i) {
        if (!i->isTag()) continue;
        if (i->tagName() != "table") continue;
        i->parseAttributes();
        auto ii = i->attributes().find("class");
        if (ii != i->attributes().end() && ii->second == "standings") {
            table_node = i;
        }
    }
    /*
    if (table_node != dom.end()) {
        cout << *table_node << endl;
    }
    */
    auto rowi = dom.begin(table_node);
    while (rowi != dom.end(table_node) && (!rowi->isTag() || rowi->tagName() != "tr")) ++rowi;
    if (rowi == dom.end(table_node)) return true;
    //cout << *rowi << endl;
    // scan the header row
    vector<string> col_names;
    for (auto coli = dom.begin(rowi); coli != dom.end(rowi); ++coli) {
        if (coli->isTag() && (coli->tagName() == "td" || coli->tagName() == "th")) {
            auto texti = dom.begin(coli);
            if (texti != dom.end(coli)) {
                //cout << texti->text() << endl;
                col_names.push_back(texti->text());
            }
        }
    }

    for (++rowi; rowi != dom.end(table_node); ++rowi) {
        if (!rowi->isTag()) continue;
        if (rowi->tagName() != "tr") continue;
        string theuser;
        int index = 0;
        for (auto coli = dom.begin(rowi); coli != dom.end(rowi); ++coli) {
            if (!coli->isTag()) continue;
            if (coli->tagName() != "td") continue;
            if (index >= int(col_names.size())) {
            } else if (col_names[index] == "Place") {
            } else if (col_names[index] == "User") {
                auto texti = dom.begin(coli);
                if (texti != dom.end(coli)) {
                    theuser = texti->text();
                }
                if (theuser == "%:") break;
                if (theuser == "Success:") break;
                if (theuser == "Total:") break;

                auto it = usergroups.find(theuser);
                if (it == usergroups.end()) {
                    usergroups.insert(make_pair(theuser, gi.get_name()));
                } else {
                    it->second.append(" ");
                    it->second.append(gi.get_name());
                }
                auto it2 = usergrsets.find(theuser);
                if (it2 == usergrsets.end()) {
                    set<string> nset;
                    nset.insert(gi.get_name());
                    usergrsets.insert(make_pair(theuser, nset));
                } else {
                    it2->second.insert(gi.get_name());
                }
            } else if (col_names[index] == "Solved") {
            } else if (col_names[index] == "Score") {
            } else {
                string text;
                int score = -1;
                CellStatus status = CellStatus::PARTIAL;
                auto texti = dom.begin(coli);
                if (texti != dom.end(coli)) {
                    if (texti->isTag()) {
                        auto ti2 = dom.begin(texti);
                        if (ti2 != dom.end(texti)) {
                            text = ti2->text();
                            status = CellStatus::FULL;
                        }
                    } else {
                        text = texti->text();
                    }
                }
                if (text == "&nbsp;") text = "";
                if (text != "") {
                    try {
                        score = stol(text);
                    } catch (...) {
                    }
                }
                if (theuser != "" && col_names[index] != "" && score >= 0) {
                    cells.insert(make_pair(CellId(theuser, col_names[index]), Cell(status, score)));
                }
            }
            ++index;
        }
    }
    return true;
}

void Course::assign_columns()
{
    // count categories
    for (int i = 0; i < int(categories.size()); ++i) {
        catinfos.insert(make_pair(categories[i].name, CategoryInfo(i, categories[i].crediting, categories[i].grader)));
    }
    for (const auto &pi : problems) {
        auto ci = catinfos.find(pi.second.get_category());
        if (ci != catinfos.end()) {
            ++ci->second.count;
            int score = pi.second.get_score();
            if (score > 0) {
                ci->second.max_score += score;
            }
        }
    }
    for (int i = 0; i < int(categories.size()); ++i) {
        CategoryInfo &cur = catinfos.find(categories[i].name)->second;
        problem_count += cur.count;
        if (cur.crediting) {
            auto gii = grade_idx.find(cur.grader);
            if (gii != grade_idx.end()) {
                GradeInfo &grade_info = grades[gii->second];
                grade_info.prob_count += cur.count;
                grade_info.max_score += cur.max_score;
            }
        }
    }
    for (int i = 1; i < int(categories.size()); ++i) {
        const CategoryInfo &prev = catinfos.find(categories[i - 1].name)->second;
        CategoryInfo &cur = catinfos.find(categories[i].name)->second;
        cur.start_pos = prev.start_pos + prev.count;
        cur.current = cur.start_pos;
    }

    // assign columns to problems
    for (auto &pi : problems) {
        auto ci = catinfos.find(pi.second.get_category());
        if (ci != catinfos.end()) {
            pi.second.set_column(ci->second.current);
            ++ci->second.current;
        }
    }

    /*
    cout << "Categories: " << endl;
    for (int i = 0; i < int(categories.size()); ++i) {
        auto ci = catinfos.find(categories[i].name);
        if (ci != catinfos.end()) {
            cout << categories[i].name << " " << ci->second.count << " " << ci->second.start_pos << " " << ci->second.max_score << " " << ci->second.current << endl;
        }
    }

    cout << "Grades: " << endl;
    for (int i = 0; i < int(grades.size()); ++i) {
        const GradeInfo &grade_info = grades[i];
        cout << grade_info.name << " " << grade_info.max_score << " " << grade_info.prob_count << endl;
    }

    cout << "Problems: " << problem_count << endl;
    for (auto &pi : problems) {
        cout << pi.second.get_name() << " " << pi.second.get_column() << endl;
    }
    */
}

struct SortByScore
{
    const Course &course;

    SortByScore(const Course &course_) : course(course_) {}

    bool operator()(const string &name1, const string &name2)
    {
        auto i1 = course.userinfos.find(name1);
        if (i1 == course.userinfos.end()) abort();
        const UserInfo &u1 = i1->second;
        auto i2 = course.userinfos.find(name2);
        if (i2 == course.userinfos.end()) abort();
        const UserInfo &u2 = i2->second;

        if (u1.total_score > u2.total_score) return true;
        if (u1.total_score < u2.total_score) return false;
        if (u1.total_prob > u2.total_prob) return true;
        if (u1.total_prob < u2.total_prob) return false;
        if (u1.group < u2.group) return true;
        if (u1.group > u2.group) return false;
        return u1.name < u2.name;
    }
};
struct SortByProblems
{
    const Course &course;

    SortByProblems(const Course &course_) : course(course_) {}

    bool operator()(const string &name1, const string &name2)
    {
        auto i1 = course.userinfos.find(name1);
        if (i1 == course.userinfos.end()) abort();
        const UserInfo &u1 = i1->second;
        auto i2 = course.userinfos.find(name2);
        if (i2 == course.userinfos.end()) abort();
        const UserInfo &u2 = i2->second;

        if (u1.total_prob > u2.total_prob) return true;
        if (u1.total_prob < u2.total_prob) return false;
        if (u1.total_score > u2.total_score) return true;
        if (u1.total_score < u2.total_score) return false;
        if (u1.group < u2.group) return true;
        if (u1.group > u2.group) return false;
        return u1.name < u2.name;
    }
};

void Course::assign_users()
{
    for (const auto &nn : usergroups) {
        userinfos.insert(make_pair(nn.first, UserInfo(nn.first, nn.second, problem_count, categories.size(), grades.size())));
    }

    for (const auto &cl : cells) {
        const auto &ci = cl.first;
        const auto &cc = cl.second;
        if (cc.get_status() == CellStatus::EMPTY) continue;
        if (cc.get_score() < 0) continue;
        const string &user = ci.get_user();
        const string &problem = ci.get_problem();
        auto uii = userinfos.find(user);
        if (uii == userinfos.end()) {
            fprintf(stderr, "userinfo '%s' not found\n", user.c_str());
            abort();
        }
        UserInfo &user_info = uii->second;
        auto pii = problems.find(problem);
        if (pii == problems.end()) {
            fprintf(stderr, "problem '%s' not found\n", problem.c_str());
            continue;
        }
        ProblemInfo &prob_info = pii->second;
        auto cii = catinfos.find(prob_info.get_category());
        if (cii == catinfos.end()) {
            fprintf(stderr, "category '%s' not found\n", prob_info.get_category().c_str());
            continue;
        }
        CategoryInfo &cat_info = cii->second;
        user_info.row[prob_info.get_column()] = cc;
        user_info.score_by_cat[cat_info.index] += cc.get_score();
        user_info.total_score += cc.get_score();
        if (cc.get_status() == CellStatus::FULL) {
            ++user_info.prob_by_cat[cat_info.index];
            ++user_info.total_prob;
        }
        auto gii = grade_idx.find(cat_info.grader);
        if (gii != grade_idx.end()) {
            //GradeInfo &grad_info = grades[gii->second];
            user_info.score_by_grad[gii->second] += cc.get_score();
            if (cc.get_status() == CellStatus::FULL) {
                ++user_info.prob_by_grad[gii->second];
            }
        }
    }

    vector<string> usernames;
    for (const auto &ui : userinfos) {
        usernames.push_back(ui.second.name);
    }
    if (sort_mode == 1) {
        sort(usernames.begin(), usernames.end(), SortByProblems(*this));
    } else {
        sort(usernames.begin(), usernames.end(), SortByScore(*this));
    }

    int serial = 0;
    for (const auto &name : usernames) {
        auto ui = userinfos.find(name);
        if (ui == userinfos.end()) abort();
        const UserInfo &u = ui->second;
        if (u.total_prob <= 0) continue;
        auto ugi = usergrsets.find(name);
        if (ugi == usergrsets.end()) abort();
        const set<string> &grps = ugi->second;
        for (const auto grpn : grps) {
            auto gi = groupidx.find(grpn);
            if (gi == groupidx.end()) abort();
            GroupInfo &g = groups[gi->second];
            group_all.add_stat(++serial, u.total_score, u.total_prob);
            g.add_stat(serial, u.total_score, u.total_prob);
        }
    }

    int best_score = 0;
    for (const auto &name : usernames) {
        auto ui = userinfos.find(name);
        if (ui == userinfos.end()) abort();
        UserInfo &u = ui->second;
        if (u.total_score > best_score)
            best_score = u.total_score;
    }
    if (best_score <= 0) best_score = 100;

    if (header_name.size() > 0) {
        cout << read_file(header_name);
    } else {
        cout << "<html>" << endl;
        cout << "<head>" << endl;
        cout << "<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">" << endl;
        cout << "<style>" << endl;
        cout << "tbody tr:nth-child(even) { background-color: #dddddd; }" << endl;
        cout << "tbody tr:nth-child(odd) { background-color: white; }" << endl;
        cout << "</style>" << endl;
        cout << "</head>" << endl;
        cout << "<body>" << endl;
        cout << "<script src=\"sorttable.js\"></script>" << endl;
    }
    cout << "<h1>Rating</h1>" << endl;
    cout << "<table class=\"sortable\" border=\"1\">" << endl;
    cout << "<thead>" << endl;
    /*
    cout << "<tr>" << endl;
    cout << "<th rowspan=\"2\">N</th>" << endl;
    cout << "<th rowspan=\"2\">Name</th>" << endl;
    cout << "<th rowspan=\"2\">Group</th>" << endl;
    cout << "<th rowspan=\"2\">Total<br/>Score</th>" << endl;
    cout << "<th rowspan=\"2\">Total<br/>Probs</th>" << endl;
    if (show_problems) {
        for (const auto &pii : problems) {
            cout << "<th rowspan=\"2\">" << pii.first << "</th>" << endl;
        }
    }
    if (show_accumulated) {
        cout << "<th rowspan=\"2\">Accum</th>" << endl;
    }
    if (!hide_summary) {
        if (!hide_marks) {
            cout << "<th rowspan=\"2\" colspan=\"" << grades.size() << "\">Marks</th>" << endl;
        }
        cout << "<th colspan=\"" << (categories.size() * 2) << "\">Categories</th>" << endl;
        if (!hide_marks) {
            cout << "<th colspan=\"" << (grades.size() * 3) << "\">Grades</th>" << endl;
        }
    }
    cout << "</tr>" << endl;
    cout << "<tr>" << endl;
    if (!hide_summary) {
        for (int i = 0; i < int(categories.size()); ++i) {
            auto ii = catinfos.find(categories[i].name);
            if (ii != catinfos.end()) {
                //const CategoryInfo &ci = ii->second;
                cout << "<th colspan=\"2\">" << categories[i].name << "</th>";
            }
        }
        if (!hide_marks) {
            for (int i = 0; i < int(grades.size()); ++i) {
                cout << "<th colspan=\"3\">" << grades[i].name << "</th>";
            }
        }
    }
    cout << "</tr>" << endl;
    */
    cout << "<tr>" << endl;
    cout << "<th>N</th>" << endl;
    cout << "<th>Name</th>" << endl;
    cout << "<th>Group</th>" << endl;
    cout << "<th title=\"Total Score\">T. S.</th>" << endl;
    if (show_percent) {
        cout << "<th>%</th>" << endl;
    }
    cout << "<th title=\"Total Problems\">T. P.</th>" << endl;
    if (show_problems) {
        for (const auto &pii : problems) {
            cout << "<th>" << pii.first << "</th>" << endl;
        }
    }
    if (show_accumulated) {
        cout << "<th>Accum</th>" << endl;
    }
    if (!hide_summary) {
        if (!hide_grades) {
            for (int i = 0; i < int(grades.size()); ++i) {
                cout << "<th>" << grades[i].name << " (" << grades[i].marks[100] << ")</th>";
            }
        }
        for (int i = 0; i < int(categories.size()); ++i) {
            auto ii = catinfos.find(categories[i].name);
            if (ii != catinfos.end()) {
                const CategoryInfo &ci = ii->second;
                cout << "<th>" << categories[i].name << " S (" << ci.max_score << ")</th>";
                cout << "<th>" << categories[i].name << " P (" << ci.count << ")</th>";
            }
        }
        if (!hide_marks) {
            for (int i = 0; i < int(grades.size()); ++i) {
                cout << "<th>" << grades[i].name << " S (" << grades[i].max_score << ")</th>";
                cout << "<th>" << grades[i].name << " P (" << grades[i].prob_count << ")</th>";
                if (!hide_grades) {
                    cout << "<th>" << grades[i].name << " M (" << grades[i].marks[100] << ")</th>";
                }
            }
        }
    }
    cout << "</tr>" << endl;
    cout << "</thead>" << endl;
    cout << "<tbody>" << endl;
    serial = 0;
    int prev_grade = -1;
    int prev_grade_2 = -1;
    string prev_grade_str;
    for (int nindex = 0; nindex < int(usernames.size()); ++nindex) {
        const auto &name = usernames[nindex];
        auto ui = userinfos.find(name);
        if (ui == userinfos.end()) abort();
        UserInfo &u = ui->second;

        cout << "<tr>" << endl;
        int cur_grade = u.total_score;
        int cur_grade_2 = u.total_prob;
        if (sort_mode == 1) {
            cur_grade = u.total_prob;
            cur_grade_2 = u.total_score;
        }
        if (!nindex || cur_grade != prev_grade || cur_grade_2 != prev_grade_2) {
            prev_grade = cur_grade;
            prev_grade_2 = cur_grade_2;
            int endind = nindex + 1;
            for (; endind < int(usernames.size()); ++endind) {
                auto nui = userinfos.find(usernames[endind]);
                if (nui == userinfos.end()) abort();
                UserInfo &nu = nui->second;
                int next_grade = nu.total_score;
                int next_grade_2 = nu.total_prob;
                if (sort_mode == 1) {
                    next_grade = nu.total_prob;
                    next_grade_2 = nu.total_score;
                }
                if (next_grade != prev_grade || next_grade_2 != prev_grade_2) break;
            }
            if (endind == nindex + 1) {
                prev_grade_str = to_string(nindex + 1);
            } else {
                prev_grade_str = to_string(nindex + 1) + "-" + to_string(endind);
            }
        }
        cout << "<td>" << prev_grade_str << "</td>";
        cout << "<td>" << u.name << "</td>";
        cout << "<td>" << u.group << "</td>";

        cout << "<td>" << u.total_score << "</td>";
        if (show_percent) {
            double pp = u.total_score * 100.0 / max_score;
            char buf[64];
            sprintf(buf, "%.2g%%", pp);
            cout << "<td>" << buf << "</td>";
        }
        cout << "<td>" << u.total_prob << "</td>";

        if (show_problems) {
            for (const auto &pii : problems) {
                const ProblemInfo &prob_info = pii.second;
                const auto &cc = u.row[prob_info.get_column()];
                cout << "<td>";
                switch (cc.get_status()) {
                case CellStatus::EMPTY:
                    cout << "&nbsp;";
                    break;
                case CellStatus::PARTIAL:
                    cout << cc.get_score();
                    break;
                case CellStatus::FULL:
                    cout << "<b>" << cc.get_score() << "</b>";
                    break;
                }
                cout << "</td>";
            }
        }

        if (!hide_summary) {
            int grad_summ = 0;
            for (int i = 0; i < int(u.score_by_grad.size()); ++i) {
                int perc1 = 0;
                int perc2 = 0;
                if (grades[i].max_score > 0) {
                    perc1 = (u.score_by_grad[i] * 100LL + grades[i].max_score - 1) / grades[i].max_score;
                }
                if (grades[i].prob_count > 0) {
                    perc2 = (u.prob_by_grad[i] * 100LL + grades[i].prob_count - 1) / grades[i].prob_count;
                }
                if (perc1 < 0) perc1 = 0;
                if (perc1 > 100) perc1 = 100;
                    int mark = grades[i].marks[perc1];
                    if (mark < 0) mark = 0;
                    u.mark_by_grad[i] = mark;
                    (void) perc2;
                    grad_summ += mark;
            }

            if (show_accumulated) {
                // FIXME: use config!!!
                const static int grad_summ_map[] =
                {
                    0, 2, 3, 5, 7, 8, 10
                };
                cout << "<td><b>" << grad_summ_map[grad_summ] << "</b></td>";
            }

            if (!hide_grades) {
                for (int i = 0; i < int(u.score_by_grad.size()); ++i) {
                    cout << "<td><b>" << u.mark_by_grad[i] << "</b></td>";
                }
            }

            for (int i = 0; i < int(u.score_by_cat.size()); ++i) {
                cout << "<td>" << u.score_by_cat[i] << "</td>";
                cout << "<td>" << u.prob_by_cat[i] << "</td>";
            }
            if (!hide_marks) {
                for (int i = 0; i < int(u.score_by_grad.size()); ++i) {
                    int perc1 = 0;
                    int perc2 = 0;
                    if (grades[i].max_score > 0) {
                        perc1 = (u.score_by_grad[i] * 100LL + grades[i].max_score - 1) / grades[i].max_score;
                    }
                    if (grades[i].prob_count > 0) {
                        perc2 = (u.prob_by_grad[i] * 100LL + grades[i].prob_count - 1) / grades[i].prob_count;
                    }
                    (void) perc2;

                    //cout << "/" << perc1 << "/" << perc2;
                    cout << "<td>" << u.score_by_grad[i] << " (" << perc1 << "%)" << "</td>";
                    cout << "<td>" << u.prob_by_grad[i] << "</td>";

                    if (!hide_grades) {
                        cout << "<td><b>" << u.mark_by_grad[i] << "</b></td>";
                    }
                }
            }
        }
        cout << "</tr>" << endl;
        cout << endl;
    }
    cout << "</tbody>" << endl;
    cout << "</table>" << endl;

    cout << "<h2>Statistics</h2>" << endl;

    cout << "<table class=\"sortable\" border=\"1\">" << endl;
    cout << "<thead>" << endl;
    cout << "<tr><th>Group</th><th>Users</th><th>Rating average</th><th>R. mediana</th><th>R. sigma</th><th>Score average</th><th>S. mediana</th><th>S. sigma</th><th>Problem average</th><th>P. mediana</th><th>P. sigma</th></tr>" << endl;
    cout << "</thead>" << endl;
    cout << "<tbody>" << endl;
    for (auto &g : groups) {
        cout << "<tr>";
        cout << "<td>" << g.get_name() << "</td>";
        cout << "<td>" << g.get_user_count() << "</td>";
        cout << "<td>" << g.get_place_avg_str() << "</td>";
        cout << "<td>" << g.get_place_mediana_str() << "</td>";
        cout << "<td>" << g.get_place_s_str() << "</td>";
        cout << "<td>" << g.get_score_avg_str() << "</td>";
        cout << "<td>" << g.get_score_mediana_str() << "</td>";
        cout << "<td>" << g.get_score_s_str() << "</td>";
        cout << "<td>" << g.get_problem_avg_str() << "</td>";
        cout << "<td>" << g.get_problem_mediana_str() << "</td>";
        cout << "<td>" << g.get_problem_s_str() << "</td>";
        cout << "</tr>" << endl;
    }
    cout << "</tbody>" << endl;

    cout << "<tfoot>" << endl;
    cout << "<tr>";
    cout << "<td>" << group_all.get_name() << "</td>";
    cout << "<td>" << group_all.get_user_count() << "</td>";
    cout << "<td>" << group_all.get_place_avg_str() << "</td>";
    cout << "<td>" << group_all.get_place_mediana_str() << "</td>";
    cout << "<td>" << group_all.get_place_s_str() << "</td>";
    cout << "<td>" << group_all.get_score_avg_str() << "</td>";
    cout << "<td>" << group_all.get_score_mediana_str() << "</td>";
    cout << "<td>" << group_all.get_score_s_str() << "</td>";
    cout << "<td>" << group_all.get_problem_avg_str() << "</td>";
    cout << "<td>" << group_all.get_problem_mediana_str() << "</td>";
    cout << "<td>" << group_all.get_problem_s_str() << "</td>";
    cout << "</tr>" << endl;
    cout << "</tfoot>" << endl;
    
    cout << "</table>" << endl;

    if (notes_name.size() > 0) {
        cout << read_file(notes_name);
    }

    cout << "<hr/>" << endl;
    cout << "<p><i>Generated " << get_current_time_str() << "</i></p>" << endl;

    if (footer_name.size() > 0) {
        cout << read_file(footer_name);
    } else {
        cout << "</body>" << endl;
        cout << "</html>" << endl;
    }
}

int main(int argc, char *argv[])
{
    Course course;

    for (int i = 1; i < argc; ++i) {
        if (!course.parse_config(argv[i])) return 1;
    }
    if (!course.process_groups()) return 1;
    course.assign_columns();
    course.assign_users();

    return 0;
}

/*
 * Local variables:
 *  c-basic-offset: 4
 * end:
 */
