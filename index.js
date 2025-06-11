import express from "express";
import bodyParser from "body-parser";

const app = express();
const port = 5000;

// Sample news items
const newsItems = [
  { date: "10 June 2025", content: "Ferrari unveils its first all-electric concept car." },
  { date: "05 June 2025", content: "Lamborghini announces limited edition V12 supercar." },
  { date: "01 June 2025", content: "Tesla overtakes BMW in Q2 EV sales worldwide." }
];

app.set("view engine", "ejs");
app.use(express.static("public"));
app.use(bodyParser.urlencoded({ extended: true }));

app.get("/", (req, res) => {
  res.render("index", { news: newsItems });
});

app.get("/news", (req, res) => {
  res.render("news", { news: newsItems });
});

app.get("/createblog", (req, res) => {
  res.render("createblog");
});

app.get("/about", (req, res) => {
  res.render("about");
});

app.get("/contact", (req, res) => {
  res.render("contactus");
});

app.listen(port, () => {
  console.log(`Server running at http://localhost:${port}`);
});
