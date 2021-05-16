from django.shortcuts import render
from django.db.models import Subquery

# Create your views here.
from django.http import HttpResponse
from rest_framework.response import Response
from rest_framework.views import APIView
from .models import Polls 
from .models import Questions  
from .models import Choices  
from .models import Answers 
from .models import AnswerText 
from .serializers import PollsSerializer
from .serializers import OnePollSerializer
from .serializers import AnswersSerializer
from .serializers import ChoicesSerializer

from .serializers import AnswerOneSerializer
from .serializers import AnswerTextSerializer
from .serializers import AnswerChoiceSerializer

def index(request):
	return HttpResponse("Hello, world. You're at the polls index.")

# list all polls, including title and description for each
class PollsView(APIView):

	def get(self, request):
		polls = Polls.objects.all()
		serializer = PollsSerializer(polls, many=True)
		return Response({"polls" : serializer.data})

# concrete id poll and questions belong to the poll with choices of answer
# poll header and list of list questions:choices
class OnePollView(APIView):

	def get(self, request, poll_id):
		# parent queryset
		questions = Questions.objects.filter(polls_parent=poll_id)
		# serializer will get choice table foreigh -> questions
		serializer = OnePollSerializer(questions,many=True)
		return Response({"poll" : serializer.data})

# all user answers for the poll 
class AnswersView(APIView):

	def get(self, request, poll_id,user_id):
		questions = Questions.objects.filter(polls_parent=poll_id)
		results = []
		for q in questions:
			serializer = OnePollSerializer(q,many=False)
			results.append(serializer.data)
			choices = Choices.objects.filter(question_parent=q)
			for c in choices:
				serializer = ChoicesSerializer(c,many=False)
				results.append(serializer.data)
				answer = Answers.objects.filter(choice_parent=c).filter(user=user_id)
				serializer = AnswersSerializer(answer,many=True)
				results.append(serializer.data)
		return Response({"answers" : results})

# POST

class AnswerTextView(APIView):
	def get(self, request, usid):
		answer = AnswerText.objects.filter(user_id=user_id)
		serializer = AnswerTextSerializer(answer, many=True)
		return Response({"answer" : serializer.data})

	def post(self, request):
		answer = request.data.get('answer')
		serializer = AnswerTextSerializer(data=answer)
		if serializer.is_valid(raise_exception=True):
			answer_saved = serializer.save()
		return Response({"success": "Anser Text  '{}' created successfully".format(answer_saved)})

class AnswerOneView(APIView):

	def get(self, request, poll_parent, user_id):
		answers = Answers.objects.filter(user=user_id)
		serializer = OneAnswersSerializer(answers, many=True)
		return Response({"answers" : serializer.data})

	def post(self, request):
		answer = request.data.get('answer')
		serializer = AnswerOneSerializer(data=answer)
		if serializer.is_valid(raise_exception=True):
			answer_saved = serializer.save()
		return Response({"success": "Anser Choice  '{}' created successfully".format(answer_saved)})

class AnswerChoiceView(APIView):
	def get(self, request, user_id):
		answer_choice = AnswerChoice.objects.filter(user_id=user_id)
		serializer = AnswerChoiceSerializer(answer_choice, many=True)
		return Response({"answer_choice" : serializer.data})

	def post(self, request):
		answer_choice = request.data.get('answer_choice')
		serializer = AnswerChoiceSerializer(data=answer_choice)
		if serializer.is_valid(raise_exception=True):
			answer_saved = serializer.save()
		return Response({"success": "Anser Choice  '{}' created successfully".format(answer_saved)})
