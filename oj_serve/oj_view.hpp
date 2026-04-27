#pragma once

#include "oj_model.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace ns_view
{
    using ns_model::Question;

    class View
    {
    private:
        struct DifficultySummary
        {
            std::size_t easy;
            std::size_t medium;
            std::size_t hard;
            std::size_t other;

            DifficultySummary()
                : easy(0)
                , medium(0)
                , hard(0)
                , other(0)
            {
            }
        };

        static std::string Escape(const std::string& input)
        {
            std::string out;
            for(char ch : input)
            {
                switch(ch)
                {
                    case '&':
                        out += "&amp;";
                        break;
                    case '<':
                        out += "&lt;";
                        break;
                    case '>':
                        out += "&gt;";
                        break;
                    case '"':
                        out += "&quot;";
                        break;
                    case '\'':
                        out += "&#39;";
                        break;
                    default:
                        out += ch;
                        break;
                }
            }
            return out;
        }

        static std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
            {
                return static_cast<char>(std::tolower(ch));
            });
            return value;
        }

        static std::string DifficultyTone(const std::string& star)
        {
            const std::string lower = ToLower(star);
            if(lower.find("easy") != std::string::npos || star.find("简单") != std::string::npos
                || star.find("入门") != std::string::npos)
            {
                return "chip--easy";
            }

            if(lower.find("medium") != std::string::npos || star.find("中等") != std::string::npos
                || star.find("普通") != std::string::npos)
            {
                return "chip--medium";
            }

            if(lower.find("hard") != std::string::npos || star.find("困难") != std::string::npos
                || star.find("挑战") != std::string::npos)
            {
                return "chip--hard";
            }

            return "chip--neutral";
        }

        static DifficultySummary Summarize(const std::vector<Question>& questions)
        {
            DifficultySummary summary;
            for(const auto& q : questions)
            {
                const std::string tone = DifficultyTone(q.star);
                if(tone == "chip--easy")
                {
                    ++summary.easy;
                }
                else if(tone == "chip--medium")
                {
                    ++summary.medium;
                }
                else if(tone == "chip--hard")
                {
                    ++summary.hard;
                }
                else
                {
                    ++summary.other;
                }
            }
            return summary;
        }

        static std::string Trim(const std::string& value)
        {
            const std::string whitespace = " \t\r\n";
            const std::string::size_type begin = value.find_first_not_of(whitespace);
            if(begin == std::string::npos)
            {
                return "";
            }

            const std::string::size_type end = value.find_last_not_of(whitespace);
            return value.substr(begin, end - begin + 1);
        }

        static std::string Join(const std::vector<std::string>& items, const std::string& delimiter)
        {
            std::ostringstream out;
            for(std::size_t i = 0; i < items.size(); ++i)
            {
                if(i > 0)
                {
                    out << delimiter;
                }
                out << items[i];
            }
            return out.str();
        }

        static std::string PublicDescription(const std::string& desc)
        {
            std::istringstream input(desc);
            std::vector<std::string> lines;
            std::string line;
            while(std::getline(input, line))
            {
                const std::string trimmed = Trim(line);
                if(trimmed == "判题方式:" || trimmed == "判题方式："
                    || trimmed == "评测方式:" || trimmed == "评测方式：")
                {
                    break;
                }

                if(trimmed.find("题目名称:") == 0 || trimmed.find("题目名称：") == 0)
                {
                    continue;
                }

                lines.push_back(line);
            }

            while(!lines.empty() && Trim(lines.back()).empty())
            {
                lines.pop_back();
            }

            std::ostringstream out;
            bool first_line = true;
            bool last_blank = true;
            for(const auto& current : lines)
            {
                const bool blank = Trim(current).empty();
                if(blank && last_blank)
                {
                    continue;
                }

                if(!first_line)
                {
                    out << "\n";
                }
                out << current;
                first_line = false;
                last_blank = blank;
            }

            if(out.str().empty())
            {
                return "请在右侧补全函数体并提交运行。";
            }
            return out.str();
        }

        static std::string ExtractFunctionSignature(const std::string& header)
        {
            std::istringstream input(header);
            std::vector<std::string> lines;
            std::string line;
            while(std::getline(input, line))
            {
                lines.push_back(line);
            }

            for(std::size_t i = lines.size(); i > 0; --i)
            {
                std::string trimmed = Trim(lines[i - 1]);
                if(trimmed.empty() || trimmed == "{" || trimmed == "}" || trimmed == "public:"
                    || trimmed == "private:" || trimmed == "protected:")
                {
                    continue;
                }

                if(trimmed.find("#include") == 0 || trimmed.find("using namespace") == 0
                    || trimmed.find("class ") == 0 || trimmed.find("struct ") == 0)
                {
                    continue;
                }

                if(trimmed.find('(') != std::string::npos && trimmed.find(')') != std::string::npos)
                {
                    if(!trimmed.empty() && trimmed[trimmed.size() - 1] == '{')
                    {
                        trimmed = Trim(trimmed.substr(0, trimmed.size() - 1));
                    }
                    return trimmed;
                }
            }

            return "请仅补全当前函数体";
        }

        static std::string ProvidedContext(const std::string& header)
        {
            std::vector<std::string> items;
            items.push_back("必要头文件");
            if(header.find("TreeNode") != std::string::npos)
            {
                items.push_back("TreeNode");
            }
            if(header.find("ListNode") != std::string::npos)
            {
                items.push_back("ListNode");
            }
            items.push_back("Solution 类");
            return Join(items, "、");
        }

        static void PageBegin(std::ostringstream& html, const std::string& title)
        {
            html << "<!DOCTYPE html><html lang='zh-CN'><head><meta charset='UTF-8'>";
            html << "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
            html << "<title>" << Escape(title) << "</title>";
            html << "<style>";
            html << R"CSS(
:root{
  --ink:#0f172a;
  --muted:#5f6f82;
  --line:rgba(148,163,184,.28);
  --panel:rgba(255,255,255,.9);
  --brand:#0ea5e9;
  --brand-strong:#0369a1;
  --accent:#fb923c;
  --success:#059669;
  --warning:#d97706;
  --danger:#dc2626;
  --code:#081222;
}
*{box-sizing:border-box;}
html{scroll-behavior:smooth;}
body{
  margin:0;
  min-height:100vh;
  color:var(--ink);
  font-family:'Avenir Next','Segoe UI','PingFang SC','Hiragino Sans GB','Microsoft YaHei',sans-serif;
  background:
    radial-gradient(circle at 0 0, rgba(14,165,233,.18), transparent 28%),
    radial-gradient(circle at 100% 0, rgba(251,146,60,.18), transparent 24%),
    linear-gradient(180deg,#07111f 0,#0c1726 26rem,#edf3f9 26rem,#eaf1f8 100%);
}
.page-shell{position:relative;}
.wrap{
  max-width:1200px;
  margin:0 auto;
  padding:32px 20px 72px;
}
.hero{
  position:relative;
  overflow:hidden;
  padding:32px;
  border-radius:32px;
  border:1px solid rgba(255,255,255,.14);
  background:linear-gradient(135deg, rgba(15,23,42,.94), rgba(8,47,73,.82));
  color:#f8fafc;
  box-shadow:0 28px 80px rgba(2,6,23,.32);
}
.hero::before{
  content:'';
  position:absolute;
  inset:auto -10% -35% auto;
  width:280px;
  height:280px;
  background:radial-gradient(circle, rgba(255,255,255,.18), transparent 62%);
  pointer-events:none;
}
.kicker{
  display:inline-flex;
  align-items:center;
  gap:10px;
  padding:8px 12px;
  border-radius:999px;
  background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.12);
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.16em;
  color:#cbd5e1;
}
.hero-grid{
  display:grid;
  grid-template-columns:minmax(0,1.5fr) minmax(280px,.85fr);
  gap:24px;
  align-items:end;
}
.hero h1{
  margin:18px 0 14px;
  font-size:clamp(2rem,4.6vw,3.4rem);
  line-height:1.05;
  letter-spacing:-.04em;
}
.hero p{
  margin:0;
  max-width:760px;
  color:rgba(226,232,240,.86);
  font-size:16px;
  line-height:1.7;
}
.stat-row,.badge-row{
  display:flex;
  flex-wrap:wrap;
  gap:12px;
  margin-top:24px;
}
.stat-card{
  min-width:140px;
  padding:16px 18px;
  border-radius:20px;
  background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.1);
}
.stat-card span{
  display:block;
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.14em;
  color:#cbd5e1;
}
.stat-card strong{
  display:block;
  margin-top:10px;
  font-size:24px;
  font-weight:700;
}
.hero-note{
  padding:20px 22px;
  border-radius:24px;
  background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.12);
  backdrop-filter:blur(10px);
}
.hero-note h2{
  margin:0 0 10px;
  font-size:20px;
}
.hero-note p{
  font-size:15px;
}
.hero-metrics{
  display:grid;
  grid-template-columns:repeat(3,minmax(0,1fr));
  gap:12px;
  margin-top:18px;
}
.metric{
  padding:14px 12px;
  border-radius:18px;
  background:rgba(15,23,42,.38);
  text-align:center;
}
.metric span{
  display:block;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
  color:#cbd5e1;
}
.metric strong{
  display:block;
  margin-top:8px;
  font-size:18px;
}
.crumb{
  display:inline-flex;
  align-items:center;
  gap:8px;
  margin-bottom:18px;
  padding:10px 14px;
  border-radius:999px;
  background:rgba(255,255,255,.08);
  border:1px solid rgba(255,255,255,.12);
  color:#e2e8f0;
  text-decoration:none;
}
.crumb:hover{background:rgba(255,255,255,.14);}
.surface,.panel{
  margin-top:24px;
  padding:24px;
  border-radius:28px;
  background:rgba(255,255,255,.88);
  border:1px solid rgba(255,255,255,.75);
  box-shadow:0 22px 50px rgba(15,23,42,.08);
  backdrop-filter:blur(12px);
}
.section-head{
  display:flex;
  justify-content:space-between;
  gap:16px;
  align-items:flex-end;
  margin-bottom:20px;
}
.section-kicker{
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.16em;
  color:#64748b;
}
.section-head h2{
  margin:6px 0 0;
  font-size:28px;
  letter-spacing:-.03em;
}
.section-tip{
  color:#64748b;
  font-size:14px;
}
.problem-grid{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(260px,1fr));
  gap:18px;
}
.problem-tools{
  display:grid;
  grid-template-columns:minmax(0,1fr) minmax(180px,220px) minmax(160px,200px);
  gap:14px;
  margin-bottom:20px;
  align-items:end;
}
.input-shell{
  display:block;
}
.input-shell span{
  display:block;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
  color:#64748b;
}
.text-input,.select-input{
  width:100%;
  margin-top:8px;
  padding:14px 16px;
  border-radius:18px;
  border:1px solid #cbd5e1;
  background:#ffffff;
  color:#0f172a;
  font-size:14px;
  outline:none;
  transition:border-color .2s ease, box-shadow .2s ease;
}
.text-input:focus,.select-input:focus{
  border-color:rgba(14,165,233,.75);
  box-shadow:0 0 0 4px rgba(14,165,233,.12);
}
.toolbar-meta{
  padding:14px 16px;
  border-radius:18px;
  background:#f8fafc;
  border:1px solid #e2e8f0;
}
.toolbar-meta span{
  display:block;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
  color:#64748b;
}
.toolbar-meta strong{
  display:block;
  margin-top:8px;
  font-size:18px;
  line-height:1.4;
}
.empty-state{
  margin-top:18px;
  padding:28px 24px;
  border-radius:22px;
  border:1px dashed #cbd5e1;
  background:linear-gradient(180deg,#f8fbff,#eef4fb);
  color:#475569;
  text-align:center;
  line-height:1.7;
}
.problem-card{
  display:block;
  padding:22px;
  border-radius:24px;
  color:inherit;
  text-decoration:none;
  background:linear-gradient(180deg,#ffffff,#f8fbff);
  border:1px solid #e2e8f0;
  box-shadow:0 12px 30px rgba(15,23,42,.05);
  transition:transform .2s ease, box-shadow .2s ease, border-color .2s ease;
}
.problem-card:hover{
  transform:translateY(-4px);
  border-color:rgba(14,165,233,.38);
  box-shadow:0 18px 42px rgba(14,165,233,.16);
}
.problem-card[hidden]{
  display:none;
}
.card-top{
  display:flex;
  justify-content:space-between;
  gap:12px;
  align-items:center;
}
.problem-number{
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.14em;
  color:#64748b;
}
.problem-title{
  margin:16px 0 18px;
  font-size:24px;
  line-height:1.2;
  letter-spacing:-.03em;
}
.chip{
  display:inline-flex;
  align-items:center;
  justify-content:center;
  padding:8px 12px;
  border-radius:999px;
  font-size:13px;
  font-weight:600;
}
.chip--easy{
  background:rgba(16,185,129,.12);
  color:#047857;
}
.chip--medium{
  background:rgba(245,158,11,.14);
  color:#b45309;
}
.chip--hard{
  background:rgba(239,68,68,.14);
  color:#b91c1c;
}
.chip--neutral{
  background:rgba(148,163,184,.18);
  color:#475569;
}
.meta-grid{
  display:grid;
  grid-template-columns:repeat(2,minmax(0,1fr));
  gap:12px;
}
.meta-item{
  padding:14px;
  border-radius:18px;
  background:#f8fafc;
  border:1px solid #e2e8f0;
}
.meta-item span{
  display:block;
  font-size:12px;
  letter-spacing:.08em;
  text-transform:uppercase;
  color:#64748b;
}
.meta-item strong{
  display:block;
  margin-top:8px;
  font-size:18px;
}
.card-footer{
  display:flex;
  justify-content:space-between;
  align-items:center;
  margin-top:20px;
  color:#0f172a;
  font-weight:600;
}
.card-footer small{
  color:#64748b;
  font-weight:500;
}
.arrow{
  color:var(--brand-strong);
  font-size:20px;
}
.dashboard{
  display:grid;
  grid-template-columns:minmax(0,1.08fr) minmax(320px,.92fr);
  gap:24px;
  margin-top:24px;
  align-items:start;
}
.info-strip{
  display:grid;
  grid-template-columns:repeat(3,minmax(0,1fr));
  gap:12px;
  margin-top:20px;
}
.info-box{
  padding:16px;
  border-radius:20px;
  background:#f8fafc;
  border:1px solid #e2e8f0;
}
.info-box span{
  display:block;
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.12em;
  color:#64748b;
}
.info-box strong{
  display:block;
  margin-top:10px;
  font-size:16px;
  line-height:1.5;
}
.mono-copy{
  font-family:'JetBrains Mono','Fira Code',Consolas,monospace;
  font-size:14px;
  word-break:break-word;
}
.statement{
  margin:0;
  padding:24px;
  border-radius:22px;
  background:linear-gradient(180deg,#f8fbff,#f1f5f9);
  border:1px solid #e2e8f0;
  white-space:pre-wrap;
  word-break:break-word;
  line-height:1.75;
  font-size:15px;
}
.context-grid{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(220px,1fr));
  gap:12px;
  margin-bottom:18px;
}
.context-card{
  padding:18px;
  border-radius:20px;
  background:linear-gradient(180deg,#f8fbff,#eef4fb);
  border:1px solid #e2e8f0;
}
.context-card span{
  display:block;
  font-size:12px;
  text-transform:uppercase;
  letter-spacing:.12em;
  color:#64748b;
}
.context-card strong{
  display:block;
  margin-top:10px;
  font-size:15px;
  line-height:1.6;
}
.workspace{
  position:sticky;
  top:24px;
}
.code-shell{
  margin-top:18px;
  border-radius:22px;
  overflow:hidden;
  background:var(--code);
  border:1px solid rgba(148,163,184,.18);
  box-shadow:inset 0 1px 0 rgba(255,255,255,.04);
}
.code-caption{
  display:flex;
  justify-content:space-between;
  gap:12px;
  align-items:center;
  padding:12px 16px;
  background:rgba(15,23,42,.72);
  color:#cbd5e1;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
}
.code-preview{
  margin:0;
  padding:18px;
  color:#dbeafe;
  background:transparent;
  white-space:pre;
  overflow:auto;
  font-family:'JetBrains Mono','Fira Code',Consolas,monospace;
  font-size:13px;
  line-height:1.65;
}
.field-label{
  display:block;
  margin:18px 0 10px;
  font-size:13px;
  letter-spacing:.12em;
  text-transform:uppercase;
  color:#64748b;
  font-weight:700;
}
.editor,.stdin-box{
  width:100%;
  resize:vertical;
  border-radius:22px;
  padding:18px;
  border:1px solid #cbd5e1;
  font-family:'JetBrains Mono','Fira Code',Consolas,monospace;
  font-size:14px;
  line-height:1.65;
  outline:none;
  transition:border-color .2s ease, box-shadow .2s ease;
}
.editor{
  min-height:280px;
  background:#07111f;
  color:#e5f0ff;
  box-shadow:inset 0 1px 0 rgba(255,255,255,.04);
}
.stdin-box{
  min-height:140px;
  background:#f8fafc;
  color:#0f172a;
}
.editor:focus,.stdin-box:focus{
  border-color:rgba(14,165,233,.75);
  box-shadow:0 0 0 4px rgba(14,165,233,.16);
}
.action-bar{
  display:flex;
  align-items:center;
  gap:12px;
  flex-wrap:wrap;
  margin-top:18px;
}
.primary-btn{
  appearance:none;
  border:0;
  border-radius:999px;
  padding:14px 22px;
  background:linear-gradient(135deg,#0ea5e9,#0369a1);
  color:#ffffff;
  font-size:15px;
  font-weight:700;
  cursor:pointer;
  box-shadow:0 16px 30px rgba(14,165,233,.28);
  transition:transform .2s ease, box-shadow .2s ease, opacity .2s ease;
}
.primary-btn:hover:not(:disabled){
  transform:translateY(-1px);
  box-shadow:0 20px 36px rgba(14,165,233,.32);
}
.primary-btn:disabled{
  cursor:not-allowed;
  opacity:.7;
}
.secondary-btn{
  appearance:none;
  border:1px solid #cbd5e1;
  border-radius:999px;
  padding:14px 20px;
  background:#ffffff;
  color:#0f172a;
  font-size:15px;
  font-weight:700;
  cursor:pointer;
  transition:transform .2s ease, box-shadow .2s ease, border-color .2s ease;
}
.secondary-btn:hover:not(:disabled){
  transform:translateY(-1px);
  border-color:rgba(14,165,233,.32);
  box-shadow:0 16px 30px rgba(148,163,184,.16);
}
.secondary-btn:disabled{
  cursor:not-allowed;
  opacity:.7;
}
.status-pill{
  display:inline-flex;
  align-items:center;
  padding:10px 14px;
  border-radius:999px;
  font-weight:700;
  font-size:13px;
}
.status-idle{
  background:#e2e8f0;
  color:#334155;
}
.status-success{
  background:rgba(5,150,105,.14);
  color:#047857;
}
.status-warning{
  background:rgba(217,119,6,.14);
  color:#b45309;
}
.status-danger{
  background:rgba(220,38,38,.14);
  color:#b91c1c;
}
.draft-note{
  margin-top:14px;
  color:#64748b;
  font-size:13px;
  line-height:1.6;
}
.output-panel{margin-top:24px;}
.result-summary{
  display:grid;
  grid-template-columns:repeat(auto-fit,minmax(140px,1fr));
  gap:12px;
}
.summary-card{
  padding:16px;
  border-radius:20px;
  background:#f8fafc;
  border:1px solid #e2e8f0;
}
.summary-card span{
  display:block;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
  color:#64748b;
}
.summary-card strong{
  display:block;
  margin-top:8px;
  font-size:15px;
  line-height:1.55;
}
.result-grid{
  display:grid;
  gap:14px;
  margin-top:18px;
}
.result-card{
  overflow:hidden;
  border-radius:22px;
  background:#09111d;
  border:1px solid rgba(148,163,184,.18);
}
.result-card--placeholder{
  background:linear-gradient(180deg,#f8fbff,#eef4fb);
  border:1px dashed #cbd5e1;
}
.result-head{
  display:flex;
  justify-content:space-between;
  gap:12px;
  align-items:center;
  padding:12px 16px;
  background:rgba(15,23,42,.72);
  color:#cbd5e1;
  font-size:12px;
  letter-spacing:.12em;
  text-transform:uppercase;
}
.result-card--placeholder .result-head{
  background:rgba(226,232,240,.7);
  color:#475569;
}
.result-content{
  margin:0;
  min-height:84px;
  padding:18px;
  white-space:pre-wrap;
  word-break:break-word;
  font-family:'JetBrains Mono','Fira Code',Consolas,monospace;
  line-height:1.7;
  font-size:13px;
  color:#e2e8f0;
}
.result-card--placeholder .result-content{
  color:#334155;
  font-family:'Avenir Next','Segoe UI','PingFang SC','Hiragino Sans GB','Microsoft YaHei',sans-serif;
  font-size:14px;
}
.watcher{
  position:fixed;
  right:24px;
  bottom:24px;
  z-index:30;
  display:flex;
  align-items:flex-end;
  gap:14px;
  pointer-events:none;
}
.watcher-note{
  position:relative;
  max-width:220px;
  padding:14px 16px;
  border-radius:20px 20px 8px 20px;
  background:rgba(255,255,255,.94);
  border:1px solid rgba(148,163,184,.28);
  box-shadow:0 20px 42px rgba(15,23,42,.14);
  color:#0f172a;
  font-size:14px;
  font-weight:700;
  line-height:1.5;
  transition:background .22s ease, border-color .22s ease, color .22s ease, transform .22s ease;
}
.watcher-note::after{
  content:'';
  position:absolute;
  right:26px;
  bottom:-10px;
  width:18px;
  height:18px;
  background:rgba(255,255,255,.94);
  border-right:1px solid rgba(148,163,184,.28);
  border-bottom:1px solid rgba(148,163,184,.28);
  transform:rotate(45deg);
}
.watcher-mascot{
  position:relative;
  width:136px;
  height:150px;
  animation:watcherFloat 3.8s ease-in-out infinite;
  transform-origin:center bottom;
}
.watcher-ear{
  position:absolute;
  top:8px;
  width:40px;
  height:48px;
  border-radius:24px 24px 12px 12px;
  background:linear-gradient(180deg,#fff8f1,#ffe0bc);
  border:2px solid rgba(15,23,42,.08);
  box-shadow:inset 0 -8px 0 rgba(251,146,60,.1);
}
.watcher-ear::after{
  content:'';
  position:absolute;
  inset:8px 10px 10px;
  border-radius:20px 20px 10px 10px;
  background:linear-gradient(180deg,#ffc8d9,#ffb0c5);
}
.watcher-ear--left{
  left:12px;
  transform:rotate(-18deg);
}
.watcher-ear--right{
  right:12px;
  transform:rotate(18deg);
}
.watcher-face{
  position:absolute;
  left:50%;
  bottom:0;
  width:126px;
  height:118px;
  transform:translateX(-50%);
  border-radius:44px 44px 38px 38px;
  background:
    radial-gradient(circle at 50% 22%, rgba(255,255,255,.86), transparent 34%),
    linear-gradient(180deg,#fffaf3,#ffe7c7);
  border:2px solid rgba(15,23,42,.08);
  box-shadow:0 24px 44px rgba(251,146,60,.22);
}
.watcher-face::before{
  content:'';
  position:absolute;
  left:50%;
  top:18px;
  width:28px;
  height:18px;
  transform:translateX(-50%);
  border-radius:18px 18px 14px 14px;
  background:rgba(255,181,197,.88);
}
.watcher-cheek{
  position:absolute;
  top:64px;
  width:22px;
  height:14px;
  border-radius:999px;
  background:rgba(255,171,196,.45);
}
.watcher-cheek--left{
  left:14px;
}
.watcher-cheek--right{
  right:14px;
}
.watcher-brow-row{
  position:absolute;
  top:34px;
  left:50%;
  display:flex;
  gap:20px;
  transform:translateX(-50%);
}
.watcher-brow{
  width:18px;
  height:6px;
  border-radius:999px;
  background:#122033;
  transform-origin:center;
  opacity:.8;
  transition:transform .22s ease, opacity .22s ease, width .22s ease;
}
.watcher-brow--left{
  transform:rotate(8deg);
}
.watcher-brow--right{
  transform:rotate(-8deg);
}
.watcher-eye-row{
  position:absolute;
  top:44px;
  left:50%;
  display:flex;
  gap:18px;
  transform:translateX(-50%);
}
.watcher-eye{
  position:relative;
  width:26px;
  height:28px;
  overflow:hidden;
  border-radius:50%;
  background:#fff;
  border:2px solid #122033;
  animation:watcherBlink 5.6s infinite;
  transform-origin:center 55%;
}
.watcher-pupil{
  position:absolute;
  left:50%;
  top:50%;
  width:11px;
  height:14px;
  margin:-7px 0 0 -5.5px;
  border-radius:50%;
  background:#122033;
  box-shadow:inset 0 -2px 0 rgba(14,165,233,.35);
  transform:translate3d(var(--pupil-x, 0px), var(--pupil-y, 0px), 0);
}
.watcher-pupil::after{
  content:'';
  position:absolute;
  top:2px;
  left:3px;
  width:3px;
  height:3px;
  border-radius:50%;
  background:rgba(255,255,255,.92);
}
.watcher-mouth{
  position:absolute;
  left:50%;
  top:76px;
  width:26px;
  height:14px;
  margin-left:-13px;
  border-bottom:3px solid #f97316;
  border-radius:0 0 16px 16px;
}
.watcher-mouth::before,.watcher-mouth::after{
  content:'';
  position:absolute;
  top:-4px;
  width:10px;
  height:10px;
  border-radius:50%;
  background:#f97316;
}
.watcher-mouth::before{
  left:1px;
}
.watcher-mouth::after{
  right:1px;
}
@keyframes watcherFloat{
  0%,100%{
    transform:translateY(0px);
  }
  50%{
    transform:translateY(-8px);
  }
}
@keyframes watcherBlink{
  0%, 44%, 48%, 100%{
    transform:scaleY(1);
  }
  46%{
    transform:scaleY(.08);
  }
}
.watcher--busy .watcher-note{
  background:rgba(255,246,221,.97);
  border-color:rgba(245,158,11,.28);
  color:#8a4b00;
}
.watcher--busy .watcher-note::after{
  background:rgba(255,246,221,.97);
  border-right-color:rgba(245,158,11,.28);
  border-bottom-color:rgba(245,158,11,.28);
}
.watcher--busy .watcher-brow--left{
  transform:rotate(18deg);
}
.watcher--busy .watcher-brow--right{
  transform:rotate(-18deg);
}
.watcher--busy .watcher-face{
  box-shadow:0 24px 44px rgba(245,158,11,.24);
}
.watcher--success .watcher-note{
  background:rgba(236,253,245,.97);
  border-color:rgba(16,185,129,.24);
  color:#0f6d48;
}
.watcher--success .watcher-note::after{
  background:rgba(236,253,245,.97);
  border-right-color:rgba(16,185,129,.24);
  border-bottom-color:rgba(16,185,129,.24);
}
.watcher--success .watcher-face{
  box-shadow:0 24px 44px rgba(16,185,129,.22);
}
.watcher--success .watcher-cheek{
  background:rgba(255,139,181,.62);
}
.watcher--success .watcher-brow--left{
  transform:rotate(-14deg) translateY(1px);
}
.watcher--success .watcher-brow--right{
  transform:rotate(14deg) translateY(1px);
}
.watcher--success .watcher-mouth{
  top:72px;
  width:34px;
  height:18px;
  margin-left:-17px;
  border-bottom-width:4px;
}
.watcher--error .watcher-note{
  background:rgba(255,241,242,.97);
  border-color:rgba(239,68,68,.2);
  color:#991b1b;
}
.watcher--error .watcher-note::after{
  background:rgba(255,241,242,.97);
  border-right-color:rgba(239,68,68,.2);
  border-bottom-color:rgba(239,68,68,.2);
}
.watcher--error .watcher-face{
  box-shadow:0 24px 44px rgba(239,68,68,.18);
}
.watcher--error .watcher-brow{
  width:20px;
  opacity:1;
}
.watcher--error .watcher-brow--left{
  transform:rotate(26deg) translateY(1px);
}
.watcher--error .watcher-brow--right{
  transform:rotate(-26deg) translateY(1px);
}
.watcher--error .watcher-eye{
  animation-duration:6.8s;
}
.watcher--error .watcher-mouth{
  top:80px;
  width:24px;
  height:0;
  margin-left:-12px;
  border-bottom:0;
  border-top:3px solid #f97316;
  border-radius:0;
}
.watcher--error .watcher-mouth::before,
.watcher--error .watcher-mouth::after{
  display:none;
}
a{color:inherit;}
@media (max-width: 980px){
  .hero-grid,.dashboard{grid-template-columns:1fr;}
  .workspace{position:static;}
}
@media (max-width: 720px){
  .wrap{padding:20px 14px 56px;}
  .hero,.surface,.panel{padding:20px;}
  .hero h1{font-size:clamp(1.8rem,11vw,2.6rem);}
  .stat-row,.badge-row{gap:10px;}
  .meta-grid,.info-strip,.hero-metrics,.problem-tools{grid-template-columns:1fr;}
  .watcher{
    right:14px;
    bottom:14px;
    gap:10px;
    align-items:center;
  }
  .watcher-note{
    max-width:150px;
    padding:10px 12px;
    font-size:12px;
  }
  .watcher-note::after{
    right:18px;
  }
  .watcher-mascot{
    width:96px;
    height:104px;
  }
  .watcher-ear{
    width:28px;
    height:34px;
  }
  .watcher-face{
    width:92px;
    height:84px;
  }
  .watcher-face::before{
    top:14px;
    width:20px;
    height:12px;
  }
  .watcher-eye-row{
    top:31px;
    gap:12px;
  }
  .watcher-brow-row{
    top:24px;
    gap:12px;
  }
  .watcher-brow{
    width:13px;
    height:4px;
  }
  .watcher-eye{
    width:19px;
    height:21px;
  }
  .watcher-pupil{
    width:8px;
    height:10px;
    margin:-5px 0 0 -4px;
  }
  .watcher-cheek{
    top:46px;
    width:16px;
    height:10px;
  }
  .watcher-mouth{
    top:56px;
    width:20px;
    height:10px;
    margin-left:-10px;
  }
  .watcher--success .watcher-mouth{
    top:53px;
    width:24px;
    height:12px;
    margin-left:-12px;
  }
  .watcher--error .watcher-mouth{
    top:58px;
    width:18px;
    margin-left:-9px;
  }
  .section-head{
    align-items:flex-start;
    flex-direction:column;
  }
  .problem-title{font-size:22px;}
}
)CSS";
            html << "</style></head><body><div class='page-shell'><div class='wrap'>";
        }

        static void PageEnd(std::ostringstream& html)
        {
            html << "</div></div></body></html>";
        }

    public:
        static void RenderAllQuestions(const std::vector<Question>& questions, std::string* html_str)
        {
            if(html_str == nullptr)
            {
                return;
            }

            const DifficultySummary summary = Summarize(questions);
            std::ostringstream html;
            PageBegin(html, "题目列表");

            html << "<section class='hero'>";
            html << "<div class='kicker'>Practice Center</div>";
            html << "<div class='hero-grid'><div>";
            html << "<h1>在线判题题库</h1>";
            html << "<p>统一浏览题目、查看资源限制，并直接进入作答页面。界面只保留做题需要的信息，入口和结果反馈都按练习流程重新整理。</p>";
            html << "<div class='stat-row'>";
            html << "<div class='stat-card'><span>题目总数</span><strong>" << questions.size() << "</strong></div>";
            html << "<div class='stat-card'><span>简单</span><strong>" << summary.easy << "</strong></div>";
            html << "<div class='stat-card'><span>中等</span><strong>" << summary.medium << "</strong></div>";
            html << "<div class='stat-card'><span>困难</span><strong>" << summary.hard << "</strong></div>";
            if(summary.other > 0)
            {
                html << "<div class='stat-card'><span>其他</span><strong>" << summary.other << "</strong></div>";
            }
            html << "</div></div>";
            html << "<div class='hero-note'><h2>练习说明</h2>";
            html << "<p>选择题目后可直接在页面内补全函数体并运行。题目页默认隐藏系统模板与内部测试细节，减少干扰，只保留解题所需上下文。</p>";
            html << "</div></div></section>";

            html << "<section class='surface'>";
            html << "<div class='section-head'><div><div class='section-kicker'>Problem Set</div><h2>题目列表</h2></div>";
            html << "<div class='section-tip'>点击任意卡片进入答题页</div></div>";
            html << "<div class='problem-tools'>";
            html << "<label class='input-shell'><span>搜索题目</span>";
            html << "<input id='search' class='text-input' type='search' placeholder='输入题号或标题关键字'></label>";
            html << "<label class='input-shell'><span>难度筛选</span>";
            html << "<select id='difficulty-filter' class='select-input'>";
            html << "<option value='all'>全部难度</option>";
            html << "<option value='chip--easy'>简单</option>";
            html << "<option value='chip--medium'>中等</option>";
            html << "<option value='chip--hard'>困难</option>";
            html << "<option value='chip--neutral'>其他</option>";
            html << "</select></label>";
            html << "<div class='toolbar-meta'><span>当前结果</span><strong><span id='visible-count'>" << questions.size()
                 << "</span> / " << questions.size() << " 道题</strong></div>";
            html << "</div>";
            html << "<div class='problem-grid'>";
            for(const auto& q : questions)
            {
                const std::string tone = DifficultyTone(q.star);
                const std::string keywords = ToLower(q.number + " " + q.title + " " + q.star);
                html << "<a class='problem-card' data-difficulty='" << tone << "' data-keywords='" << Escape(keywords)
                     << "' href='/question/" << Escape(q.number) << "'>";
                html << "<div class='card-top'><div class='problem-number'>Problem " << Escape(q.number) << "</div>";
                html << "<span class='chip " << tone << "'>" << Escape(q.star) << "</span></div>";
                html << "<div class='problem-title'>" << Escape(q.title) << "</div>";
                html << "<div class='meta-grid'>";
                html << "<div class='meta-item'><span>时间限制</span><strong>" << q.cpu_limit << " s</strong></div>";
                html << "<div class='meta-item'><span>空间限制</span><strong>" << q.mem_limit << " MB</strong></div>";
                html << "</div>";
                html << "<div class='card-footer'><small>进入练习</small><span class='arrow'>&rarr;</span></div>";
                html << "</a>";
            }
            html << "</div>";
            html << "<div id='empty-state' class='empty-state' hidden>没有匹配的题目，换个题号、标题或难度再试。</div>";
            html << "</section>";
            html << "<script>";
            html << R"JS(
const questionSearchEl = document.getElementById('search');
const questionDifficultyEl = document.getElementById('difficulty-filter');
const visibleCountEl = document.getElementById('visible-count');
const emptyStateEl = document.getElementById('empty-state');
const questionCards = Array.from(document.querySelectorAll('.problem-card'));

function applyQuestionFilters() {
  const keyword = (questionSearchEl.value || '').trim().toLowerCase();
  const difficulty = questionDifficultyEl.value;
  let visible = 0;

  questionCards.forEach(function(card) {
    const cardKeywords = card.dataset.keywords || '';
    const keywordMatched = !keyword || cardKeywords.indexOf(keyword) !== -1;
    const difficultyMatched = difficulty === 'all' || card.dataset.difficulty === difficulty;
    const matched = keywordMatched && difficultyMatched;
    card.hidden = !matched;
    if (matched) {
      visible += 1;
    }
  });

  visibleCountEl.textContent = String(visible);
  emptyStateEl.hidden = visible !== 0;
}

questionSearchEl.addEventListener('input', applyQuestionFilters);
questionDifficultyEl.addEventListener('change', applyQuestionFilters);
applyQuestionFilters();
)JS";
            html << "</script>";

            PageEnd(html);
            *html_str = html.str();
        }

        static void RenderQuestion(const Question& q, std::string* html_str)
        {
            if(html_str == nullptr)
            {
                return;
            }

            const std::string statement = PublicDescription(q.desc);
            const std::string signature = ExtractFunctionSignature(q.header);
            const std::string provided_context = ProvidedContext(q.header);
            std::ostringstream html;
            PageBegin(html, q.title);

            html << "<section class='hero'>";
            html << "<a class='crumb' href='/all_questions'>&larr; 返回题目列表</a>";
            html << "<div class='hero-grid'><div>";
            html << "<div class='kicker'>Problem " << Escape(q.number) << "</div>";
            html << "<h1>" << Escape(q.title) << "</h1>";
            html << "<p>阅读题意后，在右侧作答区补全函数体并直接运行。系统会自动完成编译和校验，页面只展示做题必需的信息与结果反馈。</p>";
            html << "<div class='badge-row'>";
            html << "<span class='chip " << DifficultyTone(q.star) << "'>" << Escape(q.star) << "</span>";
            html << "<span class='chip chip--neutral'>CPU " << q.cpu_limit << " s</span>";
            html << "<span class='chip chip--neutral'>Memory " << q.mem_limit << " MB</span>";
            html << "</div></div>";
            html << "<div class='hero-note'><h2>作答提示</h2>";
            html << "<p>只需填写函数体，不需要编写 main、输入解析或额外测试代码。提交后会自动返回编译状态、运行结果和必要的错误提示。</p>";
            html << "<div class='hero-metrics'>";
            html << "<div class='metric'><span>题号</span><strong>" << Escape(q.number) << "</strong></div>";
            html << "<div class='metric'><span>CPU</span><strong>" << q.cpu_limit << " s</strong></div>";
            html << "<div class='metric'><span>Memory</span><strong>" << q.mem_limit << " MB</strong></div>";
            html << "</div></div></div></section>";

            html << "<div class='dashboard'>";
            html << "<section class='panel'>";
            html << "<div class='section-head'><div><div class='section-kicker'>Statement</div><h2>题目描述</h2></div></div>";
            html << "<pre class='statement'>" << Escape(statement) << "</pre>";
            html << "<div class='info-strip'>";
            html << "<div class='info-box'><span>函数签名</span><strong class='mono-copy'>" << Escape(signature) << "</strong></div>";
            html << "<div class='info-box'><span>系统已提供</span><strong>" << Escape(provided_context) << "</strong></div>";
            html << "<div class='info-box'><span>运行入口</span><strong>点击按钮或使用 Ctrl/Cmd + Enter</strong></div>";
            html << "</div></section>";

            html << "<aside class='workspace'>";
            html << "<section class='panel'>";
            html << "<div class='section-head'><div><div class='section-kicker'>Workspace</div><h2>作答区</h2></div>";
            html << "<div class='section-tip'>只填写函数体</div></div>";
            html << "<div class='context-grid'>";
            html << "<div class='context-card'><span>当前函数</span><strong class='mono-copy'>" << Escape(signature)
                 << "</strong></div>";
            html << "<div class='context-card'><span>预置上下文</span><strong>" << Escape(provided_context)
                 << "</strong></div>";
            html << "</div>";
            html << "<label class='field-label' for='code'>函数体</label>";
            html << "<textarea id='code' class='editor' spellcheck='false'>        // 在这里补全核心逻辑\n</textarea>";
            html << "<div class='section-tip'>无需编写 main、读写输入或手动准备测试代码。</div>";
            html << "<div class='action-bar'><button id='run-btn' class='primary-btn' type='button'>提交运行</button>";
            html << "<button id='reset-btn' class='secondary-btn' type='button'>重置草稿</button>";
            html << "<span class='status-pill status-idle' id='status'>等待提交</span></div>";
            html << "<div class='draft-note'>浏览器会自动保存当前题目的代码草稿，刷新页面或下次打开会继续恢复。</div>";
            html << "</section>";

            html << "<section class='panel output-panel'>";
            html << "<div class='section-head'><div><div class='section-kicker'>Output</div><h2>判题结果</h2></div></div>";
            html << "<div id='result-summary' class='result-summary'></div>";
            html << "<div id='result-grid' class='result-grid'>";
            html << "<div class='result-card result-card--placeholder'><div class='result-head'><span>判题结果</span><span>等待提交</span></div>";
            html << "<pre class='result-content'>提交后会在这里展示判题结果。</pre></div></div>";
            html << "</section></aside></div>";
            html << "<div class='watcher' aria-hidden='true'>";
            html << "<div class='watcher-note'>不要作弊哦，我在监视你</div>";
            html << "<div class='watcher-mascot'>";
            html << "<div class='watcher-ear watcher-ear--left'></div>";
            html << "<div class='watcher-ear watcher-ear--right'></div>";
            html << "<div class='watcher-face'>";
            html << "<div class='watcher-brow-row'>";
            html << "<div class='watcher-brow watcher-brow--left'></div>";
            html << "<div class='watcher-brow watcher-brow--right'></div>";
            html << "</div>";
            html << "<div class='watcher-eye-row'>";
            html << "<div class='watcher-eye'><div class='watcher-pupil'></div></div>";
            html << "<div class='watcher-eye'><div class='watcher-pupil'></div></div>";
            html << "</div>";
            html << "<div class='watcher-cheek watcher-cheek--left'></div>";
            html << "<div class='watcher-cheek watcher-cheek--right'></div>";
            html << "<div class='watcher-mouth'></div>";
            html << "</div></div></div>";

            html << "<script>";
            html << "const endpoint='/judge/" << Escape(q.number) << "';";
            html << R"JS(
const btn = document.getElementById('run-btn');
const resetBtn = document.getElementById('reset-btn');
const statusEl = document.getElementById('status');
const codeEl = document.getElementById('code');
const resultSummaryEl = document.getElementById('result-summary');
const resultGridEl = document.getElementById('result-grid');
const watcherEl = document.querySelector('.watcher');
const watcherNoteEl = document.querySelector('.watcher-note');
const watcherEyes = Array.from(document.querySelectorAll('.watcher-eye'));
const starterCode = codeEl.defaultValue;
const draftKey = 'oj-draft:' + endpoint;

function setStatus(text, tone) {
  statusEl.textContent = text;
  statusEl.className = 'status-pill ' + tone;
}

function setWatcherMood(mood, message) {
  if (!watcherEl || !watcherNoteEl) {
    return;
  }

  watcherEl.classList.remove('watcher--idle', 'watcher--busy', 'watcher--success', 'watcher--error');
  watcherEl.classList.add('watcher--' + mood);
  watcherNoteEl.textContent = message;
}

function moveWatcherEyes(clientX, clientY) {
  watcherEyes.forEach(function(eye) {
    const pupil = eye.querySelector('.watcher-pupil');
    if (!pupil) {
      return;
    }

    const rect = eye.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    const deltaX = clientX - centerX;
    const deltaY = clientY - centerY;
    const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY) || 1;
    const maxOffset = Math.max(3, Math.min(rect.width, rect.height) * 0.22);
    const offsetX = Math.max(-maxOffset, Math.min(maxOffset, deltaX / distance * maxOffset));
    const offsetY = Math.max(-maxOffset, Math.min(maxOffset, deltaY / distance * maxOffset));

    pupil.style.setProperty('--pupil-x', offsetX.toFixed(2) + 'px');
    pupil.style.setProperty('--pupil-y', offsetY.toFixed(2) + 'px');
  });
}

function resetWatcherEyes() {
  watcherEyes.forEach(function(eye) {
    const pupil = eye.querySelector('.watcher-pupil');
    if (!pupil) {
      return;
    }
    pupil.style.setProperty('--pupil-x', '0px');
    pupil.style.setProperty('--pupil-y', '0px');
  });
}

function analyzeChecks(stdout) {
  if (!stdout) {
    return null;
  }

  const lines = String(stdout).split(/\r?\n/).filter(function(line) {
    return line.trim() !== '';
  });
  let passed = 0;
  let failed = 0;

  lines.forEach(function(line) {
    if (line.indexOf('... OK') !== -1) {
      passed += 1;
    } else if (line.indexOf('... Failed') !== -1) {
      failed += 1;
    }
  });

  const total = passed + failed;
  if (total === 0) {
    return null;
  }

  return {
    passed: passed,
    failed: failed,
    total: total
  };
}

function verdictFromPayload(payload) {
  if (!payload || typeof payload !== 'object') {
    return '服务异常';
  }

  if (payload.compiler_error || payload.status === -3) {
    return '编译错误';
  }
  if (payload.status === -1) {
    return '提交为空';
  }
  if (payload.status === -2) {
    return '运行失败';
  }
  if (typeof payload.status === 'number' && payload.status > 0) {
    return '运行异常';
  }

  const checks = analyzeChecks(payload.stdout);
  if (checks) {
    return checks.failed === 0 ? '通过' : '未通过';
  }
  if (payload.status === 0) {
    return '已完成';
  }
  return '处理中';
}

function toneFromVerdict(verdict) {
  if (verdict === '通过' || verdict === '已完成') {
    return 'status-success';
  }
  if (verdict === '提交为空') {
    return 'status-warning';
  }
  return 'status-danger';
}

function checkSummaryText(checks) {
  if (!checks) {
    return '未返回校验明细';
  }
  return String(checks.passed) + ' / ' + String(checks.total) + ' 通过';
}

function createSummaryCard(item) {
  const card = document.createElement('div');
  card.className = 'summary-card';
  const label = document.createElement('span');
  label.textContent = item.label;
  const value = document.createElement('strong');
  value.textContent = item.value;
  card.appendChild(label);
  card.appendChild(value);
  return card;
}

function renderSummary(items) {
  resultSummaryEl.innerHTML = '';
  items.forEach(function(item) {
    resultSummaryEl.appendChild(createSummaryCard(item));
  });
}

function createResultCard(section) {
  const card = document.createElement('div');
  card.className = 'result-card' + (section.placeholder ? ' result-card--placeholder' : '');

  const head = document.createElement('div');
  head.className = 'result-head';

  const title = document.createElement('span');
  title.textContent = section.title;
  const tag = document.createElement('span');
  tag.textContent = section.tag;
  head.appendChild(title);
  head.appendChild(tag);

  const content = document.createElement('pre');
  content.className = 'result-content';
  content.textContent = section.content;

  card.appendChild(head);
  card.appendChild(content);
  return card;
}

function renderSections(sections) {
  resultGridEl.innerHTML = '';
  sections.forEach(function(section) {
    resultGridEl.appendChild(createResultCard(section));
  });
}

function renderIdleState(message) {
  const draftState = codeEl.value === starterCode ? '未修改' : '已保存';
  renderSummary([
    { label: '草稿状态', value: draftState },
    { label: '快捷键', value: 'Ctrl/Cmd + Enter' },
    { label: '运行方式', value: '在线提交运行' }
  ]);
  setWatcherMood('idle', '不要作弊哦，我在监视你');
  renderSections([
    {
      title: '判题结果',
      tag: 'Idle',
      content: message,
      placeholder: true
    }
  ]);
}

function persistDraft() {
  try {
    localStorage.setItem(draftKey, codeEl.value);
  } catch (e) {
  }
}

function restoreDraft() {
  try {
    const saved = localStorage.getItem(draftKey);
    if (saved !== null) {
      codeEl.value = saved;
    }
  } catch (e) {
  }
}

function resetDraft() {
  codeEl.value = starterCode;
  try {
    localStorage.removeItem(draftKey);
  } catch (e) {
  }
  setStatus('草稿已重置', 'status-idle');
  renderIdleState('草稿已重置，提交后会在这里展示判题结果。');
}

function renderPayload(payload, rawText, httpStatus) {
  if (!payload || typeof payload !== 'object') {
    setWatcherMood('error', '先别急，我这边都看不懂返回结果了');
    renderSummary([
      { label: '状态', value: '服务异常' },
      { label: '说明', value: '返回格式异常' },
      { label: '建议', value: '请稍后重试' }
    ]);
    renderSections([
      {
        title: '结果说明',
        tag: 'Error',
        content: '服务返回了无法识别的内容，暂时无法展示判题结果。',
        placeholder: true
      }
    ]);
    return;
  }

  const checks = analyzeChecks(payload.stdout);
  const verdict = verdictFromPayload(payload);
  const outputState = payload.compiler_error ? '编译未通过' : (payload.stdout ? '有结果详情' : '无额外输出');
  if (verdict === '通过' || verdict === '已完成') {
    setWatcherMood('success', '哼哼，这次写得不错，继续保持');
  } else if (verdict === '编译错误') {
    setWatcherMood('error', '别糊弄我，编译都没过呢');
  } else if (verdict === '未通过') {
    setWatcherMood('error', '我盯到了，结果还没对上哦');
  } else if (verdict === '提交为空') {
    setWatcherMood('busy', '你还没认真写呢，我先继续盯着');
  } else {
    setWatcherMood('error', '我正在看着你，先把问题改掉');
  }
  renderSummary([
    { label: '判题结果', value: verdict },
    { label: '校验情况', value: checkSummaryText(checks) },
    { label: '结果说明', value: payload.reason || '已完成' },
    { label: '详情状态', value: outputState }
  ]);

  const sections = [];
  if (payload.compiler_error) {
    sections.push({ title: '编译信息', tag: 'Compile', content: payload.compiler_error });
  } else if (payload.stdout) {
    sections.push({
      title: checks ? '校验详情' : '运行输出',
      tag: checks ? (checks.failed === 0 ? 'Passed' : 'Failed') : 'Output',
      content: payload.stdout
    });
  }
  if (payload.stderr && verdict !== '通过') {
    sections.push({ title: '错误详情', tag: 'Error', content: payload.stderr });
  }
  if (sections.length === 0) {
    sections.push({
      title: '结果说明',
      tag: 'Info',
      content: payload.reason || '本次提交已完成。',
      placeholder: true
    });
  }
  renderSections(sections);
}

async function judge() {
  if (btn.disabled) {
    return;
  }

  btn.disabled = true;
  resetBtn.disabled = true;
  btn.textContent = '运行中...';
  persistDraft();
  setWatcherMood('busy', '不要切出去翻答案哦，我正盯着你');
  renderSummary([
    { label: '状态', value: '正在判题' },
    { label: '提交方式', value: '在线提交' },
    { label: '草稿', value: '已保存到本地' }
  ]);
  renderSections([
    {
      title: '判题结果',
      tag: 'Running',
      content: '正在提交并等待编译运行结果...',
      placeholder: true
    }
  ]);
  setStatus('正在判题', 'status-warning');

  const payload = {
    code: codeEl.value,
    input: ''
  };

  try {
    const resp = await fetch(endpoint, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const httpStatus = resp.status;
    const text = await resp.text();
    let parsed = null;
    try {
      parsed = JSON.parse(text);
    } catch (e) {
    }

    renderPayload(parsed, text, httpStatus);
    if (!resp.ok) {
      setStatus('请求失败', 'status-danger');
      return;
    }

    if (parsed && typeof parsed === 'object') {
      const verdict = verdictFromPayload(parsed);
      setStatus(verdict, toneFromVerdict(verdict));
    } else {
      setStatus('请求完成', 'status-success');
    }
  } catch (err) {
    setWatcherMood('error', '网络开小差了，但我还是在盯着你');
    renderSummary([
      { label: '状态', value: '网络异常' },
      { label: '说明', value: '暂时无法连接判题服务' },
      { label: '建议', value: '请稍后重试' }
    ]);
    renderSections([
      {
        title: '结果说明',
        tag: 'Error',
        content: '未能连接到判题服务，请确认服务正在运行后重试。',
        placeholder: true
      }
    ]);
    setStatus('网络异常', 'status-danger');
  } finally {
    btn.disabled = false;
    resetBtn.disabled = false;
    btn.textContent = '再次运行';
  }
}

restoreDraft();
renderIdleState(codeEl.value === starterCode
  ? '提交后会在这里展示判题结果。'
  : '已恢复上次草稿，提交后会在这里展示判题结果。');

btn.addEventListener('click', judge);
resetBtn.addEventListener('click', resetDraft);
codeEl.addEventListener('input', persistDraft);
window.addEventListener('mousemove', function(event) {
  moveWatcherEyes(event.clientX, event.clientY);
});
window.addEventListener('mouseleave', resetWatcherEyes);
codeEl.addEventListener('keydown', function(event) {
  if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
    event.preventDefault();
    judge();
  }
});
setWatcherMood('idle', '不要作弊哦，我在监视你');
resetWatcherEyes();
)JS";
            html << "</script>";

            PageEnd(html);
            *html_str = html.str();
        }
    };
}
